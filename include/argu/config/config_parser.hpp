#pragma once

/// @file argu/config/config_parser.hpp
/// @brief Configuration file parsing (INI, TOML-like, JSON-like)

#include <argu/core/error.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace argu {

    /// Configuration value types
    using ConfigValue = std::variant<std::string, std::vector<std::string>>;

    /// Configuration data from a file
    class ConfigData {
      public:
        /// Get a single value
        std::optional<std::string> get(const std::string &key) const {
            auto it = m_values.find(key);
            if (it == m_values.end())
                return std::nullopt;

            if (std::holds_alternative<std::string>(it->second)) {
                return std::get<std::string>(it->second);
            }
            auto &vec = std::get<std::vector<std::string>>(it->second);
            if (!vec.empty())
                return vec.front();
            return std::nullopt;
        }

        /// Get multiple values (for arrays)
        std::vector<std::string> get_many(const std::string &key) const {
            auto it = m_values.find(key);
            if (it == m_values.end())
                return {};

            if (std::holds_alternative<std::string>(it->second)) {
                return {std::get<std::string>(it->second)};
            }
            return std::get<std::vector<std::string>>(it->second);
        }

        /// Check if key exists
        bool contains(const std::string &key) const { return m_values.find(key) != m_values.end(); }

        /// Get all keys
        std::vector<std::string> keys() const {
            std::vector<std::string> result;
            result.reserve(m_values.size());
            for (const auto &[k, v] : m_values) {
                result.push_back(k);
            }
            return result;
        }

        /// Set a value
        void set(const std::string &key, const std::string &value) { m_values[key] = value; }

        /// Set multiple values
        void set(const std::string &key, const std::vector<std::string> &values) { m_values[key] = values; }

        /// Append to a value (creates array if needed)
        void append(const std::string &key, const std::string &value) {
            auto it = m_values.find(key);
            if (it == m_values.end()) {
                m_values[key] = std::vector<std::string>{value};
            } else if (std::holds_alternative<std::string>(it->second)) {
                std::vector<std::string> vec = {std::get<std::string>(it->second), value};
                m_values[key] = vec;
            } else {
                std::get<std::vector<std::string>>(it->second).push_back(value);
            }
        }

      private:
        std::map<std::string, ConfigValue> m_values;
    };

    /// Configuration file format
    enum class ConfigFormat {
        Auto, ///< Detect from file extension
        Ini,  ///< INI format (key=value, [sections])
        Toml, ///< TOML-like format
        Json, ///< Simple JSON format
    };

    /// Configuration file parser
    class ConfigParser {
      public:
        /// Parse configuration from file
        static ConfigData parse_file(const std::string &path, ConfigFormat format = ConfigFormat::Auto) {
            std::ifstream file(path);
            if (!file.is_open()) {
                throw ConfigFileError(path, "cannot open file");
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string content = buffer.str();

            if (format == ConfigFormat::Auto) {
                format = detect_format(path);
            }

            switch (format) {
            case ConfigFormat::Ini:
            case ConfigFormat::Toml:
                return parse_ini(content, path);
            case ConfigFormat::Json:
                return parse_json(content, path);
            default:
                return parse_ini(content, path);
            }
        }

        /// Parse configuration from string
        static ConfigData parse_string(const std::string &content, ConfigFormat format = ConfigFormat::Ini) {
            switch (format) {
            case ConfigFormat::Json:
                return parse_json(content, "<string>");
            default:
                return parse_ini(content, "<string>");
            }
        }

      private:
        static ConfigFormat detect_format(const std::string &path) {
            auto dot_pos = path.rfind('.');
            if (dot_pos == std::string::npos)
                return ConfigFormat::Ini;

            std::string ext = path.substr(dot_pos + 1);
            for (auto &c : ext)
                c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

            if (ext == "json")
                return ConfigFormat::Json;
            if (ext == "toml")
                return ConfigFormat::Toml;
            if (ext == "ini" || ext == "cfg" || ext == "conf")
                return ConfigFormat::Ini;

            return ConfigFormat::Ini;
        }

        static std::string trim(const std::string &s) {
            auto start = s.find_first_not_of(" \t\n\r");
            if (start == std::string::npos)
                return "";
            auto end = s.find_last_not_of(" \t\n\r");
            return s.substr(start, end - start + 1);
        }

        static ConfigData parse_ini(const std::string &content, const std::string &filename) {
            ConfigData data;
            std::istringstream stream(content);
            std::string line;
            std::string current_section;
            int line_num = 0;

            while (std::getline(stream, line)) {
                ++line_num;
                line = trim(line);

                // Skip empty lines and comments
                if (line.empty() || line[0] == '#' || line[0] == ';')
                    continue;

                // Section header
                if (line[0] == '[') {
                    auto end = line.find(']');
                    if (end == std::string::npos) {
                        throw ConfigFileError(filename, "unterminated section at line " + std::to_string(line_num));
                    }
                    current_section = trim(line.substr(1, end - 1));
                    continue;
                }

                // Key-value pair
                auto eq_pos = line.find('=');
                if (eq_pos == std::string::npos) {
                    // Try colon separator (TOML style)
                    eq_pos = line.find(':');
                }
                if (eq_pos == std::string::npos) {
                    throw ConfigFileError(filename, "invalid syntax at line " + std::to_string(line_num));
                }

                std::string key = trim(line.substr(0, eq_pos));
                std::string value = trim(line.substr(eq_pos + 1));

                // Apply section prefix
                if (!current_section.empty()) {
                    key = current_section + "." + key;
                }

                // Handle quoted values
                if (!value.empty()) {
                    if ((value.front() == '"' && value.back() == '"') ||
                        (value.front() == '\'' && value.back() == '\'')) {
                        value = value.substr(1, value.size() - 2);
                    }
                }

                // Handle arrays (value, value, value or [value, value])
                if (!value.empty() && value.front() == '[' && value.back() == ']') {
                    value = value.substr(1, value.size() - 2);
                    std::vector<std::string> values;
                    std::istringstream vs(value);
                    std::string item;
                    while (std::getline(vs, item, ',')) {
                        item = trim(item);
                        if ((item.front() == '"' && item.back() == '"') ||
                            (item.front() == '\'' && item.back() == '\'')) {
                            item = item.substr(1, item.size() - 2);
                        }
                        values.push_back(item);
                    }
                    data.set(key, values);
                } else if (value.find(',') != std::string::npos) {
                    // Comma-separated values without brackets
                    std::vector<std::string> values;
                    std::istringstream vs(value);
                    std::string item;
                    while (std::getline(vs, item, ',')) {
                        values.push_back(trim(item));
                    }
                    data.set(key, values);
                } else {
                    data.set(key, value);
                }
            }

            return data;
        }

        static ConfigData parse_json(const std::string &content, const std::string &filename) {
            ConfigData data;

            // Simple JSON parser (handles basic object with string/array values)
            std::size_t pos = 0;
            auto skip_ws = [&]() {
                while (pos < content.size() && std::isspace(static_cast<unsigned char>(content[pos])))
                    ++pos;
            };

            auto parse_string = [&]() -> std::string {
                if (content[pos] != '"') {
                    throw ConfigFileError(filename, "expected '\"' at position " + std::to_string(pos));
                }
                ++pos;
                std::string result;
                while (pos < content.size() && content[pos] != '"') {
                    if (content[pos] == '\\' && pos + 1 < content.size()) {
                        ++pos;
                        switch (content[pos]) {
                        case 'n':
                            result += '\n';
                            break;
                        case 't':
                            result += '\t';
                            break;
                        case 'r':
                            result += '\r';
                            break;
                        case '\\':
                            result += '\\';
                            break;
                        case '"':
                            result += '"';
                            break;
                        default:
                            result += content[pos];
                            break;
                        }
                    } else {
                        result += content[pos];
                    }
                    ++pos;
                }
                if (pos >= content.size()) {
                    throw ConfigFileError(filename, "unterminated string");
                }
                ++pos; // Skip closing quote
                return result;
            };

            skip_ws();
            if (pos >= content.size() || content[pos] != '{') {
                throw ConfigFileError(filename, "expected '{' at start of JSON");
            }
            ++pos;

            while (pos < content.size()) {
                skip_ws();
                if (content[pos] == '}')
                    break;

                // Parse key
                std::string key = parse_string();
                skip_ws();

                if (content[pos] != ':') {
                    throw ConfigFileError(filename, "expected ':' after key");
                }
                ++pos;
                skip_ws();

                // Parse value (string or array)
                if (content[pos] == '"') {
                    data.set(key, parse_string());
                } else if (content[pos] == '[') {
                    ++pos;
                    std::vector<std::string> values;
                    while (pos < content.size() && content[pos] != ']') {
                        skip_ws();
                        if (content[pos] == ']')
                            break;
                        values.push_back(parse_string());
                        skip_ws();
                        if (content[pos] == ',')
                            ++pos;
                    }
                    ++pos; // Skip ]
                    data.set(key, values);
                } else if (content[pos] == 't' || content[pos] == 'f') {
                    // Boolean
                    if (content.substr(pos, 4) == "true") {
                        data.set(key, "true");
                        pos += 4;
                    } else if (content.substr(pos, 5) == "false") {
                        data.set(key, "false");
                        pos += 5;
                    }
                } else if (std::isdigit(static_cast<unsigned char>(content[pos])) || content[pos] == '-') {
                    // Number
                    std::size_t start = pos;
                    while (pos < content.size() &&
                           (std::isdigit(static_cast<unsigned char>(content[pos])) || content[pos] == '.' ||
                            content[pos] == '-' || content[pos] == 'e' || content[pos] == 'E')) {
                        ++pos;
                    }
                    data.set(key, content.substr(start, pos - start));
                } else {
                    throw ConfigFileError(filename, "unexpected character at position " + std::to_string(pos));
                }

                skip_ws();
                if (content[pos] == ',')
                    ++pos;
            }

            return data;
        }
    };

} // namespace argu
