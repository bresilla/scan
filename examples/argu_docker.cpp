/// @file argu_docker.cpp
/// @brief Docker-style CLI example demonstrating external subcommands and complex argument patterns
///
/// This example shows how to build a CLI similar to Docker with:
/// - External subcommands (docker-compose, docker-buildx style plugins)
/// - Deep subcommand chains (docker container ls)
/// - Global options propagating to subcommands
/// - Complex argument relationships

#include <argu/argu.hpp>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char *argv[]) {
    // Global options
    std::string config_path;
    std::string context;
    std::string host;
    std::string log_level = "info";
    bool debug = false;
    bool tls = false;

    // Container options
    std::string container_name;
    bool all_containers = false;
    bool quiet = false;
    std::string format;
    int last_n = 0;
    std::vector<std::string> filters;

    // Image options
    std::string image_name;
    std::string tag = "latest";
    std::string dockerfile = "Dockerfile";
    std::vector<std::string> build_args;
    bool no_cache = false;
    bool pull = false;

    // Run options
    bool detach = false;
    bool interactive = false;
    bool tty = false;
    bool rm = false;
    std::string workdir;
    std::vector<std::string> env_vars;
    std::vector<std::string> volumes;
    std::vector<std::string> ports;
    std::string network;
    std::string restart_policy;
    std::string memory_limit;
    std::string cpu_limit;

    auto cmd = argu::Command("docker")
        .version("24.0.7")
        .about("A self-sufficient runtime for containers")
        .long_about(R"(
Docker is a platform for developers and sysadmins to develop, deploy, and run
applications with containers. The use of Linux containers to deploy applications
is called containerization.

Manage containers, images, networks, and volumes with ease.
)")
        .color(argu::ColorMode::Auto)
        .theme(argu::HelpTheme::colorful())
        .allow_external_subcommands(true)  // Allow docker-compose, docker-buildx, etc.
        .allow_subcommand_prefix(true)
        .suggest_threshold(3)

        // Global options (propagate to all subcommands)
        .arg(argu::Arg("config")
            .short_name('c')
            .long_name("config")
            .help("Location of client config files")
            .value_of(config_path)
            .value_name("PATH")
            .hint_file()
            .env("DOCKER_CONFIG")
            .global())

        .arg(argu::Arg("context")
            .long_name("context")
            .help("Name of the context to use")
            .value_of(context)
            .env("DOCKER_CONTEXT")
            .global())

        .arg(argu::Arg("host")
            .short_name('H')
            .long_name("host")
            .help("Daemon socket to connect to")
            .value_of(host)
            .value_name("HOST")
            .env("DOCKER_HOST")
            .global())

        .arg(argu::Arg("log-level")
            .short_name('l')
            .long_name("log-level")
            .help("Set the logging level")
            .value_of(log_level)
            .choices({"debug", "info", "warn", "error", "fatal"})
            .default_value("info")
            .global())

        .arg(argu::Arg("debug")
            .short_name('D')
            .long_name("debug")
            .help("Enable debug mode")
            .flag(debug)
            .global())

        .arg(argu::Arg("tls")
            .long_name("tls")
            .help("Use TLS; implied by --tlsverify")
            .flag(tls)
            .global())

        // =====================================================================
        // CONTAINER management subcommand
        // =====================================================================
        .subcommand(
            argu::Command("container")
                .about("Manage containers")
                .alias("containers")
                .allow_subcommand_prefix(true)

                // docker container ls
                .subcommand(
                    argu::Command("ls")
                        .about("List containers")
                        .alias("list")
                        .alias("ps")
                        .arg(argu::Arg("all")
                            .short_name('a')
                            .long_name("all")
                            .help("Show all containers (default shows just running)")
                            .flag(all_containers))
                        .arg(argu::Arg("quiet")
                            .short_name('q')
                            .long_name("quiet")
                            .help("Only display container IDs")
                            .flag(quiet))
                        .arg(argu::Arg("format")
                            .long_name("format")
                            .help("Format output using a custom template")
                            .value_of(format)
                            .choices({"table", "json", "{{.ID}}", "{{.Names}}"}))
                        .arg(argu::Arg("last")
                            .short_name('n')
                            .long_name("last")
                            .help("Show n last created containers")
                            .value_of(last_n)
                            .validate(argu::validators::non_negative()))
                        .arg(argu::Arg("filter")
                            .short_name('f')
                            .long_name("filter")
                            .help("Filter output based on conditions")
                            .takes_one_or_more()
                            .value_of(filters))
                )

                // docker container run
                .subcommand(
                    argu::Command("run")
                        .about("Create and run a new container from an image")
                        .arg(argu::Arg("image")
                            .positional()
                            .help("Image to run")
                            .value_of(image_name)
                            .required())
                        .arg(argu::Arg("command")
                            .positional()
                            .help("Command to run")
                            .takes_multiple())
                        .arg(argu::Arg("detach")
                            .short_name('d')
                            .long_name("detach")
                            .help("Run container in background")
                            .flag(detach))
                        .arg(argu::Arg("interactive")
                            .short_name('i')
                            .long_name("interactive")
                            .help("Keep STDIN open")
                            .flag(interactive))
                        .arg(argu::Arg("tty")
                            .short_name('t')
                            .long_name("tty")
                            .help("Allocate a pseudo-TTY")
                            .flag(tty))
                        .arg(argu::Arg("rm")
                            .long_name("rm")
                            .help("Automatically remove container when it exits")
                            .flag(rm))
                        .arg(argu::Arg("name")
                            .long_name("name")
                            .help("Assign a name to the container")
                            .value_of(container_name))
                        .arg(argu::Arg("env")
                            .short_name('e')
                            .long_name("env")
                            .help("Set environment variables")
                            .takes_one_or_more()
                            .value_of(env_vars))
                        .arg(argu::Arg("volume")
                            .short_name('v')
                            .long_name("volume")
                            .help("Bind mount a volume")
                            .takes_one_or_more()
                            .value_of(volumes))
                        .arg(argu::Arg("publish")
                            .short_name('p')
                            .long_name("publish")
                            .help("Publish container ports to the host")
                            .takes_one_or_more()
                            .value_of(ports))
                        .arg(argu::Arg("network")
                            .long_name("network")
                            .help("Connect container to a network")
                            .value_of(network))
                        .arg(argu::Arg("workdir")
                            .short_name('w')
                            .long_name("workdir")
                            .help("Working directory inside the container")
                            .value_of(workdir))
                        .arg(argu::Arg("restart")
                            .long_name("restart")
                            .help("Restart policy")
                            .value_of(restart_policy)
                            .choices({"no", "on-failure", "always", "unless-stopped"}))
                        .arg(argu::Arg("memory")
                            .short_name('m')
                            .long_name("memory")
                            .help("Memory limit (e.g., 512m, 2g)")
                            .value_of(memory_limit)
                            .validate(argu::validators::size()))
                        .arg(argu::Arg("cpus")
                            .long_name("cpus")
                            .help("Number of CPUs")
                            .value_of(cpu_limit))
                )

                // docker container stop
                .subcommand(
                    argu::Command("stop")
                        .about("Stop one or more running containers")
                        .arg(argu::Arg("containers")
                            .positional()
                            .help("Container name or ID")
                            .required()
                            .takes_one_or_more())
                        .arg(argu::Arg("time")
                            .short_name('t')
                            .long_name("time")
                            .help("Seconds to wait before killing")
                            .default_value("10")
                            .validate(argu::validators::non_negative()))
                )

                // docker container rm
                .subcommand(
                    argu::Command("rm")
                        .about("Remove one or more containers")
                        .arg(argu::Arg("containers")
                            .positional()
                            .help("Container name or ID")
                            .required()
                            .takes_one_or_more())
                        .arg(argu::Arg("force")
                            .short_name('f')
                            .long_name("force")
                            .help("Force removal of running container"))
                        .arg(argu::Arg("volumes")
                            .short_name('v')
                            .long_name("volumes")
                            .help("Remove anonymous volumes"))
                )

                // docker container logs
                .subcommand(
                    argu::Command("logs")
                        .about("Fetch the logs of a container")
                        .arg(argu::Arg("container")
                            .positional()
                            .help("Container name or ID")
                            .required())
                        .arg(argu::Arg("follow")
                            .short_name('f')
                            .long_name("follow")
                            .help("Follow log output"))
                        .arg(argu::Arg("tail")
                            .long_name("tail")
                            .help("Number of lines to show from end")
                            .default_value("all"))
                        .arg(argu::Arg("timestamps")
                            .short_name('t')
                            .long_name("timestamps")
                            .help("Show timestamps"))
                        .arg(argu::Arg("since")
                            .long_name("since")
                            .help("Show logs since timestamp or duration")
                            .validate(argu::validators::duration()))
                )

                // docker container exec
                .subcommand(
                    argu::Command("exec")
                        .about("Execute a command in a running container")
                        .arg(argu::Arg("container")
                            .positional()
                            .help("Container name or ID")
                            .required())
                        .arg(argu::Arg("command")
                            .positional()
                            .help("Command to execute")
                            .required()
                            .takes_one_or_more())
                        .arg(argu::Arg("detach")
                            .short_name('d')
                            .long_name("detach")
                            .help("Detached mode"))
                        .arg(argu::Arg("interactive")
                            .short_name('i')
                            .long_name("interactive")
                            .help("Keep STDIN open"))
                        .arg(argu::Arg("tty")
                            .short_name('t')
                            .long_name("tty")
                            .help("Allocate a pseudo-TTY"))
                        .arg(argu::Arg("env")
                            .short_name('e')
                            .long_name("env")
                            .help("Set environment variables")
                            .takes_one_or_more())
                        .arg(argu::Arg("workdir")
                            .short_name('w')
                            .long_name("workdir")
                            .help("Working directory"))
                )
        )

        // =====================================================================
        // IMAGE management subcommand
        // =====================================================================
        .subcommand(
            argu::Command("image")
                .about("Manage images")
                .alias("images")
                .allow_subcommand_prefix(true)

                // docker image ls
                .subcommand(
                    argu::Command("ls")
                        .about("List images")
                        .alias("list")
                        .arg(argu::Arg("all")
                            .short_name('a')
                            .long_name("all")
                            .help("Show all images"))
                        .arg(argu::Arg("quiet")
                            .short_name('q')
                            .long_name("quiet")
                            .help("Only show image IDs"))
                        .arg(argu::Arg("filter")
                            .short_name('f')
                            .long_name("filter")
                            .help("Filter output")
                            .takes_one_or_more())
                        .arg(argu::Arg("format")
                            .long_name("format")
                            .help("Format output"))
                )

                // docker image build
                .subcommand(
                    argu::Command("build")
                        .about("Build an image from a Dockerfile")
                        .arg(argu::Arg("path")
                            .positional()
                            .help("Build context path")
                            .default_value("."))
                        .arg(argu::Arg("tag")
                            .short_name('t')
                            .long_name("tag")
                            .help("Name and optionally a tag (name:tag)")
                            .value_of(tag)
                            .takes_one_or_more())
                        .arg(argu::Arg("file")
                            .short_name('f')
                            .long_name("file")
                            .help("Name of the Dockerfile")
                            .value_of(dockerfile)
                            .hint_file())
                        .arg(argu::Arg("build-arg")
                            .long_name("build-arg")
                            .help("Set build-time variables")
                            .takes_one_or_more()
                            .value_of(build_args))
                        .arg(argu::Arg("no-cache")
                            .long_name("no-cache")
                            .help("Do not use cache")
                            .flag(no_cache))
                        .arg(argu::Arg("pull")
                            .long_name("pull")
                            .help("Always pull newer version of base image")
                            .flag(pull))
                        .arg(argu::Arg("target")
                            .long_name("target")
                            .help("Set target build stage"))
                        .arg(argu::Arg("platform")
                            .long_name("platform")
                            .help("Set platform (linux/amd64, linux/arm64)"))
                )

                // docker image pull
                .subcommand(
                    argu::Command("pull")
                        .about("Download an image from a registry")
                        .arg(argu::Arg("image")
                            .positional()
                            .help("Image name")
                            .required())
                        .arg(argu::Arg("all-tags")
                            .short_name('a')
                            .long_name("all-tags")
                            .help("Download all tagged images"))
                        .arg(argu::Arg("platform")
                            .long_name("platform")
                            .help("Set platform"))
                )

                // docker image push
                .subcommand(
                    argu::Command("push")
                        .about("Upload an image to a registry")
                        .arg(argu::Arg("image")
                            .positional()
                            .help("Image name")
                            .required())
                        .arg(argu::Arg("all-tags")
                            .short_name('a')
                            .long_name("all-tags")
                            .help("Push all tags"))
                )

                // docker image rm
                .subcommand(
                    argu::Command("rm")
                        .about("Remove one or more images")
                        .alias("rmi")
                        .arg(argu::Arg("images")
                            .positional()
                            .help("Image name or ID")
                            .required()
                            .takes_one_or_more())
                        .arg(argu::Arg("force")
                            .short_name('f')
                            .long_name("force")
                            .help("Force removal"))
                )
        )

        // =====================================================================
        // NETWORK management subcommand
        // =====================================================================
        .subcommand(
            argu::Command("network")
                .about("Manage networks")
                .allow_subcommand_prefix(true)

                .subcommand(argu::Command("ls").about("List networks").alias("list"))
                .subcommand(
                    argu::Command("create")
                        .about("Create a network")
                        .arg(argu::Arg("name").positional().help("Network name").required())
                        .arg(argu::Arg("driver").short_name('d').long_name("driver")
                            .help("Driver").default_value("bridge"))
                )
                .subcommand(
                    argu::Command("rm")
                        .about("Remove networks")
                        .arg(argu::Arg("networks").positional().help("Network names").required().takes_one_or_more())
                )
                .subcommand(
                    argu::Command("inspect")
                        .about("Display network details")
                        .arg(argu::Arg("network").positional().help("Network name").required())
                )
        )

        // =====================================================================
        // VOLUME management subcommand
        // =====================================================================
        .subcommand(
            argu::Command("volume")
                .about("Manage volumes")
                .allow_subcommand_prefix(true)

                .subcommand(argu::Command("ls").about("List volumes").alias("list"))
                .subcommand(
                    argu::Command("create")
                        .about("Create a volume")
                        .arg(argu::Arg("name").positional().help("Volume name"))
                        .arg(argu::Arg("driver").short_name('d').long_name("driver")
                            .help("Driver").default_value("local"))
                )
                .subcommand(
                    argu::Command("rm")
                        .about("Remove volumes")
                        .arg(argu::Arg("volumes").positional().help("Volume names").required().takes_one_or_more())
                        .arg(argu::Arg("force").short_name('f').long_name("force").help("Force removal"))
                )
                .subcommand(
                    argu::Command("inspect")
                        .about("Display volume details")
                        .arg(argu::Arg("volume").positional().help("Volume name").required())
                )
        )

        // =====================================================================
        // SYSTEM subcommand
        // =====================================================================
        .subcommand(
            argu::Command("system")
                .about("Manage Docker")
                .allow_subcommand_prefix(true)

                .subcommand(argu::Command("df").about("Show disk usage"))
                .subcommand(argu::Command("info").about("Display system-wide information"))
                .subcommand(
                    argu::Command("prune")
                        .about("Remove unused data")
                        .arg(argu::Arg("all").short_name('a').long_name("all").help("Remove all unused data"))
                        .arg(argu::Arg("force").short_name('f').long_name("force").help("Do not prompt"))
                        .arg(argu::Arg("volumes").long_name("volumes").help("Prune volumes"))
                )
        )

        // =====================================================================
        // Top-level shortcuts (aliases to container commands)
        // =====================================================================
        .subcommand(
            argu::Command("ps")
                .about("List containers (shortcut for 'container ls')")
                .arg(argu::Arg("all").short_name('a').long_name("all").help("Show all containers"))
                .arg(argu::Arg("quiet").short_name('q').long_name("quiet").help("Only display IDs"))
        )

        .subcommand(
            argu::Command("run")
                .about("Create and run a new container (shortcut)")
                .arg(argu::Arg("image").positional().help("Image").required())
                .arg(argu::Arg("command").positional().help("Command").takes_multiple())
                .arg(argu::Arg("detach").short_name('d').long_name("detach").help("Run in background"))
                .arg(argu::Arg("interactive").short_name('i').long_name("interactive").help("Keep STDIN open"))
                .arg(argu::Arg("tty").short_name('t').long_name("tty").help("Allocate a TTY"))
                .arg(argu::Arg("rm").long_name("rm").help("Remove container when it exits"))
                .arg(argu::Arg("name").long_name("name").help("Container name"))
        )

        .subcommand(
            argu::Command("build")
                .about("Build an image (shortcut for 'image build')")
                .arg(argu::Arg("path").positional().help("Build context").default_value("."))
                .arg(argu::Arg("tag").short_name('t').long_name("tag").help("Tag").takes_one_or_more())
                .arg(argu::Arg("file").short_name('f').long_name("file").help("Dockerfile").hint_file())
        )

        .subcommand(
            argu::Command("pull")
                .about("Download an image (shortcut for 'image pull')")
                .arg(argu::Arg("image").positional().help("Image name").required())
        )

        .subcommand(
            argu::Command("push")
                .about("Upload an image (shortcut for 'image push')")
                .arg(argu::Arg("image").positional().help("Image name").required())
        );

    // Parse arguments
    auto result = cmd.parse(argc, argv);

    if (!result.success() || !result.message().empty()) {
        return result.exit();
    }

    auto& matches = cmd.matches();

    // Show global options if debug mode
    if (debug) {
        std::cout << "=== Debug Mode ===\n";
        std::cout << "Log level: " << log_level << "\n";
        if (!host.empty()) std::cout << "Host: " << host << "\n";
        if (!context.empty()) std::cout << "Context: " << context << "\n";
        if (!config_path.empty()) std::cout << "Config: " << config_path << "\n";
        std::cout << "\n";
    }

    // Handle external subcommands (docker-compose, docker-buildx, etc.)
    if (matches.is_external_subcommand()) {
        auto sub = matches.subcommand();
        std::cout << "External plugin detected: docker-" << *sub << "\n";
        std::cout << "Would execute: docker-" << *sub;
        for (const auto& arg : matches.external_args()) {
            std::cout << " " << arg;
        }
        std::cout << "\n";
        return 0;
    }

    // Display command chain
    auto chain = matches.subcommand_chain();
    if (!chain.empty()) {
        std::cout << "Command: docker";
        for (const auto& sub : chain) {
            std::cout << " " << sub;
        }
        std::cout << "\n\n";

        // Handle specific commands
        if (chain[0] == "container" && chain.size() > 1) {
            if (chain[1] == "ls" || chain[1] == "list" || chain[1] == "ps") {
                std::cout << "Listing containers";
                if (all_containers) std::cout << " (all)";
                if (quiet) std::cout << " (quiet)";
                if (!format.empty()) std::cout << " (format: " << format << ")";
                std::cout << "\n";
            }
            else if (chain[1] == "run") {
                std::cout << "Running container from image: " << image_name << "\n";
                if (detach) std::cout << "  Mode: detached\n";
                if (interactive) std::cout << "  Mode: interactive\n";
                if (tty) std::cout << "  TTY: allocated\n";
                if (rm) std::cout << "  Auto-remove: yes\n";
                if (!container_name.empty()) std::cout << "  Name: " << container_name << "\n";
                if (!network.empty()) std::cout << "  Network: " << network << "\n";
                if (!memory_limit.empty()) std::cout << "  Memory: " << memory_limit << "\n";
                for (const auto& v : volumes) std::cout << "  Volume: " << v << "\n";
                for (const auto& p : ports) std::cout << "  Port: " << p << "\n";
                for (const auto& e : env_vars) std::cout << "  Env: " << e << "\n";
            }
        }
        else if (chain[0] == "image" && chain.size() > 1) {
            if (chain[1] == "build") {
                std::cout << "Building image\n";
                std::cout << "  Tag: " << tag << "\n";
                std::cout << "  Dockerfile: " << dockerfile << "\n";
                if (no_cache) std::cout << "  No cache: yes\n";
                if (pull) std::cout << "  Pull: yes\n";
                for (const auto& arg : build_args) std::cout << "  Build arg: " << arg << "\n";
            }
        }
        else if (chain[0] == "ps") {
            std::cout << "Listing containers (via shortcut)\n";
        }
        else if (chain[0] == "run") {
            std::cout << "Running container (via shortcut)\n";
        }
    } else {
        std::cout << "No subcommand specified. Use 'docker --help' for usage.\n";
    }

    return 0;
}
