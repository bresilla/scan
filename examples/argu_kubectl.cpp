/// @file argu_kubectl.cpp
/// @brief kubectl-style CLI example demonstrating deep subcommand chains (4+ levels)
///
/// This example shows how to build a complex CLI with deeply nested subcommands,
/// similar to kubectl's structure (e.g., kubectl config view users).

#include <argu/argu.hpp>
#include <iostream>
#include <vector>

int main(int argc, char *argv[]) {
    // Variables for global options
    bool verbose = false;
    std::string kubeconfig;
    std::string context;
    std::string namespace_name = "default";
    std::string output_format = "text";

    // Variables for specific commands
    std::string resource_name;
    std::string container_name;
    bool all_namespaces = false;
    bool watch = false;
    std::string label_selector;
    int tail_lines = -1;
    bool follow = false;
    std::string timeout = "30s";

    // Build the kubectl-like command structure
    auto cmd = argu::Command("kubectl")
        .version("1.28.0")
        .about("Kubernetes command-line tool")
        .long_about(R"(
kubectl controls the Kubernetes cluster manager.

Find more information at: https://kubernetes.io/docs/reference/kubectl/
)")
        .color(argu::ColorMode::Auto)
        .theme(argu::HelpTheme::colorful())
        .allow_subcommand_prefix(true)  // Allow "kubectl conf" for "kubectl config"
        .suggest_threshold(3)

        // Global options (available to all subcommands)
        .arg(argu::Arg("verbose")
            .short_name('v')
            .long_name("verbose")
            .help("Enable verbose output")
            .flag(verbose)
            .global())

        .arg(argu::Arg("kubeconfig")
            .long_name("kubeconfig")
            .help("Path to the kubeconfig file")
            .value_of(kubeconfig)
            .value_name("FILE")
            .hint_file()
            .env("KUBECONFIG")
            .global())

        .arg(argu::Arg("context")
            .long_name("context")
            .help("The name of the kubeconfig context to use")
            .value_of(context)
            .global())

        .arg(argu::Arg("namespace")
            .short_name('n')
            .long_name("namespace")
            .help("Kubernetes namespace")
            .value_of(namespace_name)
            .default_value("default")
            .global())

        .arg(argu::Arg("output")
            .short_name('o')
            .long_name("output")
            .help("Output format")
            .value_of(output_format)
            .choices({"text", "json", "yaml", "wide", "name"})
            .default_value("text")
            .global())

        // =====================================================================
        // GET subcommand (kubectl get pods, kubectl get deployments, etc.)
        // =====================================================================
        .subcommand(
            argu::Command("get")
                .about("Display one or many resources")
                .allow_subcommand_prefix(true)

                .arg(argu::Arg("all-namespaces")
                    .short_name('A')
                    .long_name("all-namespaces")
                    .help("List resources across all namespaces")
                    .flag(all_namespaces))

                .arg(argu::Arg("watch")
                    .short_name('w')
                    .long_name("watch")
                    .help("Watch for changes")
                    .flag(watch))

                .arg(argu::Arg("selector")
                    .short_name('l')
                    .long_name("selector")
                    .help("Selector (label query) to filter on")
                    .value_of(label_selector)
                    .value_name("SELECTOR"))

                // Resource type subcommands
                .subcommand(argu::Command("pods").about("List pods").alias("pod").alias("po"))
                .subcommand(argu::Command("deployments").about("List deployments").alias("deploy"))
                .subcommand(argu::Command("services").about("List services").alias("svc"))
                .subcommand(argu::Command("nodes").about("List nodes").alias("node"))
                .subcommand(argu::Command("namespaces").about("List namespaces").alias("ns"))
                .subcommand(argu::Command("configmaps").about("List configmaps").alias("cm"))
                .subcommand(argu::Command("secrets").about("List secrets"))
        )

        // =====================================================================
        // DESCRIBE subcommand
        // =====================================================================
        .subcommand(
            argu::Command("describe")
                .about("Show details of a specific resource")
                .arg(argu::Arg("resource")
                    .positional()
                    .help("Resource type")
                    .required())
                .arg(argu::Arg("name")
                    .positional()
                    .help("Resource name"))
        )

        // =====================================================================
        // LOGS subcommand
        // =====================================================================
        .subcommand(
            argu::Command("logs")
                .about("Print the logs for a container in a pod")
                .arg(argu::Arg("pod")
                    .positional()
                    .help("Pod name")
                    .value_of(resource_name)
                    .required())

                .arg(argu::Arg("container")
                    .short_name('c')
                    .long_name("container")
                    .help("Container name")
                    .value_of(container_name))

                .arg(argu::Arg("follow")
                    .short_name('f')
                    .long_name("follow")
                    .help("Follow log output")
                    .flag(follow))

                .arg(argu::Arg("tail")
                    .long_name("tail")
                    .help("Lines of recent log to display")
                    .value_of(tail_lines)
                    .default_value("-1")
                    .validate(argu::validators::range(-1, 10000)))

                .arg(argu::Arg("since")
                    .long_name("since")
                    .help("Show logs since duration (e.g., 5s, 2m, 3h)")
                    .value_of(timeout)
                    .validate(argu::validators::duration()))
        )

        // =====================================================================
        // CONFIG subcommand (4-level deep: kubectl config view users)
        // =====================================================================
        .subcommand(
            argu::Command("config")
                .about("Modify kubeconfig files")
                .allow_subcommand_prefix(true)
                .subcommand_required(true)

                // kubectl config view
                .subcommand(
                    argu::Command("view")
                        .about("Display merged kubeconfig settings")
                        .allow_subcommand_prefix(true)

                        // 4th level: kubectl config view users
                        .subcommand(
                            argu::Command("users")
                                .about("Display users from kubeconfig")
                        )
                        // 4th level: kubectl config view clusters
                        .subcommand(
                            argu::Command("clusters")
                                .about("Display clusters from kubeconfig")
                        )
                        // 4th level: kubectl config view contexts
                        .subcommand(
                            argu::Command("contexts")
                                .about("Display contexts from kubeconfig")
                        )
                )

                // kubectl config get-contexts
                .subcommand(
                    argu::Command("get-contexts")
                        .about("Display one or many contexts")
                        .arg(argu::Arg("name")
                            .positional()
                            .help("Context name (optional)"))
                )

                // kubectl config use-context
                .subcommand(
                    argu::Command("use-context")
                        .about("Set the current-context in kubeconfig")
                        .arg(argu::Arg("context-name")
                            .positional()
                            .help("Context name")
                            .required())
                )

                // kubectl config set-context
                .subcommand(
                    argu::Command("set-context")
                        .about("Set a context entry in kubeconfig")
                        .arg(argu::Arg("name")
                            .positional()
                            .help("Context name")
                            .required())
                        .arg(argu::Arg("cluster")
                            .long_name("cluster")
                            .help("Cluster name"))
                        .arg(argu::Arg("user")
                            .long_name("user")
                            .help("User name"))
                        .arg(argu::Arg("namespace")
                            .long_name("namespace")
                            .help("Namespace"))
                )

                // kubectl config current-context
                .subcommand(
                    argu::Command("current-context")
                        .about("Display the current-context")
                )
        )

        // =====================================================================
        // APPLY subcommand
        // =====================================================================
        .subcommand(
            argu::Command("apply")
                .about("Apply a configuration to a resource")
                .arg(argu::Arg("filename")
                    .short_name('f')
                    .long_name("filename")
                    .help("Filename, directory, or URL to files")
                    .value_name("FILE")
                    .hint_file()
                    .required())
                .arg(argu::Arg("dry-run")
                    .long_name("dry-run")
                    .help("Simulate the operation")
                    .choices({"none", "client", "server"})
                    .default_value("none"))
        )

        // =====================================================================
        // DELETE subcommand
        // =====================================================================
        .subcommand(
            argu::Command("delete")
                .about("Delete resources")
                .arg(argu::Arg("resource")
                    .positional()
                    .help("Resource type")
                    .required())
                .arg(argu::Arg("name")
                    .positional()
                    .help("Resource name"))
                .arg(argu::Arg("force")
                    .long_name("force")
                    .help("Force deletion"))
                .arg(argu::Arg("grace-period")
                    .long_name("grace-period")
                    .help("Grace period in seconds")
                    .default_value("30"))
        )

        // =====================================================================
        // EXEC subcommand
        // =====================================================================
        .subcommand(
            argu::Command("exec")
                .about("Execute a command in a container")
                .arg(argu::Arg("pod")
                    .positional()
                    .help("Pod name")
                    .required())
                .arg(argu::Arg("command")
                    .positional()
                    .help("Command to execute")
                    .takes_one_or_more())
                .arg(argu::Arg("container")
                    .short_name('c')
                    .long_name("container")
                    .help("Container name"))
                .arg(argu::Arg("stdin")
                    .short_name('i')
                    .long_name("stdin")
                    .help("Pass stdin to the container"))
                .arg(argu::Arg("tty")
                    .short_name('t')
                    .long_name("tty")
                    .help("Allocate a TTY"))
        )

        // =====================================================================
        // ROLLOUT subcommand (3 levels: kubectl rollout status deployment)
        // =====================================================================
        .subcommand(
            argu::Command("rollout")
                .about("Manage the rollout of a resource")
                .allow_subcommand_prefix(true)
                .subcommand_required(true)

                .subcommand(
                    argu::Command("status")
                        .about("Show the status of the rollout")
                        .subcommand(argu::Command("deployment").alias("deploy"))
                        .subcommand(argu::Command("daemonset").alias("ds"))
                        .subcommand(argu::Command("statefulset").alias("sts"))
                )

                .subcommand(
                    argu::Command("history")
                        .about("View rollout history")
                        .subcommand(argu::Command("deployment").alias("deploy"))
                )

                .subcommand(
                    argu::Command("undo")
                        .about("Undo a previous rollout")
                        .subcommand(argu::Command("deployment").alias("deploy"))
                )

                .subcommand(
                    argu::Command("restart")
                        .about("Restart a resource")
                        .subcommand(argu::Command("deployment").alias("deploy"))
                )
        )

        // =====================================================================
        // CLUSTER-INFO subcommand
        // =====================================================================
        .subcommand(
            argu::Command("cluster-info")
                .about("Display cluster info")
                .subcommand(argu::Command("dump").about("Dump cluster state for debugging"))
        )

        // Hidden debug command
        .subcommand(
            argu::Command("debug-internal")
                .about("Internal debugging commands")
                .hidden()
        );

    // Parse arguments
    auto result = cmd.parse(argc, argv);

    if (!result.success() || !result.message().empty()) {
        return result.exit();
    }

    auto& matches = cmd.matches();

    // Display parsed information
    std::cout << "\n";

    // Show global options if verbose
    if (verbose) {
        std::cout << "=== Global Options ===\n";
        std::cout << "Verbose:    yes\n";
        std::cout << "Kubeconfig: " << (kubeconfig.empty() ? "(default)" : kubeconfig) << "\n";
        std::cout << "Context:    " << (context.empty() ? "(current)" : context) << "\n";
        std::cout << "Namespace:  " << namespace_name << "\n";
        std::cout << "Output:     " << output_format << "\n";
        std::cout << "\n";
    }

    // Show subcommand chain
    auto chain = matches.subcommand_chain();
    if (!chain.empty()) {
        std::cout << "Command: kubectl";
        for (const auto& sub : chain) {
            std::cout << " " << sub;
        }
        std::cout << "\n\n";

        // Handle specific commands
        if (chain[0] == "get") {
            if (chain.size() > 1) {
                std::cout << "Listing " << chain[1];
                if (all_namespaces) {
                    std::cout << " across all namespaces";
                } else {
                    std::cout << " in namespace '" << namespace_name << "'";
                }
                if (!label_selector.empty()) {
                    std::cout << " with selector '" << label_selector << "'";
                }
                if (watch) {
                    std::cout << " (watching for changes)";
                }
                std::cout << "\n";
            }
        }
        else if (chain[0] == "logs") {
            std::cout << "Fetching logs for pod '" << resource_name << "'";
            if (!container_name.empty()) {
                std::cout << " container '" << container_name << "'";
            }
            if (follow) {
                std::cout << " (following)";
            }
            if (tail_lines > 0) {
                std::cout << " (last " << tail_lines << " lines)";
            }
            std::cout << "\n";
        }
        else if (chain[0] == "config") {
            if (chain.size() >= 2) {
                if (chain[1] == "view") {
                    if (chain.size() >= 3) {
                        std::cout << "Viewing " << chain[2] << " from kubeconfig\n";
                    } else {
                        std::cout << "Viewing merged kubeconfig\n";
                    }
                }
                else if (chain[1] == "get-contexts") {
                    std::cout << "Listing contexts\n";
                }
                else if (chain[1] == "use-context") {
                    std::cout << "Switching context\n";
                }
                else if (chain[1] == "current-context") {
                    std::cout << "Current context: my-cluster\n";
                }
            }
        }
        else if (chain[0] == "rollout") {
            if (chain.size() >= 3) {
                std::cout << "Rollout " << chain[1] << " for " << chain[2] << "\n";
            }
        }
    } else {
        std::cout << "No subcommand specified. Use 'kubectl --help' for usage.\n";
    }

    return 0;
}
