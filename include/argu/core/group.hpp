#pragma once

/// @file argu/core/group.hpp
/// @brief Argument group definitions (mutually exclusive, required together, etc.)

#include <argu/core/types.hpp>

#include <string>
#include <vector>

namespace argu {

    /// Argument group - groups arguments together for display and/or constraints
    class ArgGroup {
      public:
        explicit ArgGroup(std::string name, GroupType type = GroupType::Normal)
            : m_name(std::move(name)), m_type(type) {}

        /// Set the group name (displayed in help)
        ArgGroup &name(std::string n) {
            m_name = std::move(n);
            return *this;
        }

        /// Set the group type
        ArgGroup &type(GroupType t) {
            m_type = t;
            return *this;
        }

        /// Make this group mutually exclusive (only one arg can be used)
        ArgGroup &mutually_exclusive() {
            m_type = GroupType::MutuallyExclusive;
            return *this;
        }

        /// Make this group require all arguments together
        ArgGroup &required_together() {
            m_type = GroupType::RequiredTogether;
            return *this;
        }

        /// Make this group require at least one argument
        ArgGroup &at_least_one() {
            m_type = GroupType::AtLeastOne;
            return *this;
        }

        /// Mark the entire group as required
        ArgGroup &required(bool req = true) {
            m_required = req;
            return *this;
        }

        /// Add an argument to this group by name
        ArgGroup &arg(std::string arg_name) {
            m_args.push_back(std::move(arg_name));
            return *this;
        }

        /// Add multiple arguments
        ArgGroup &args(std::initializer_list<std::string> arg_names) {
            for (const auto &a : arg_names) {
                m_args.push_back(a);
            }
            return *this;
        }

        /// Hide this group from help
        ArgGroup &hidden(bool h = true) {
            m_hidden = h;
            return *this;
        }

        /// Set help text for this group
        ArgGroup &help(std::string text) {
            m_help = std::move(text);
            return *this;
        }

        // Getters
        const std::string &get_name() const { return m_name; }
        GroupType get_type() const { return m_type; }
        bool is_required() const { return m_required; }
        bool is_hidden() const { return m_hidden; }
        const std::string &get_help() const { return m_help; }
        const std::vector<std::string> &get_args() const { return m_args; }

        /// Check if an argument is in this group
        bool contains(const std::string &arg_name) const {
            for (const auto &a : m_args) {
                if (a == arg_name)
                    return true;
            }
            return false;
        }

      private:
        std::string m_name;
        GroupType m_type = GroupType::Normal;
        std::vector<std::string> m_args;
        std::string m_help;
        bool m_required = false;
        bool m_hidden = false;
    };

} // namespace argu
