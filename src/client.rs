// Module dedicated to the cli.
use clap::{arg, Command};

pub fn cli() -> Command {
    Command::new("bulgogi")
    .about("A build system orchestrator for CubeSat projects")
    .subcommand_required(true)
    .arg_required_else_help(true)
    .allow_external_subcommands(true)
    .subcommand(
        Command::new("init")
            .about("Initializes bulgogi project directory")
    )
    .subcommand(
        Command::new("spawn")
            .about("Spawns project directory structure.")
    )
    .subcommand(
        Command::new("module")
        .about("Manage project modules")
        .arg_required_else_help(true)
        .subcommand(
            Command::new("add")
            .about("Adds a new module to the bulgogi project")
            .arg(arg!(<MODULE> "Path to the module in question"))
            .arg(
                arg!(<TARGET> "Which target to add the module to")
                .default_value("default")
                .required(false)
            )
            .arg(arg!(--create))
        )
        .subcommand(
            Command::new("rm")
            .about("Removed a module from the bulgogi project")
            .alias("remove")
            .arg(arg!(<PATH> "Path to the module in question"))
            .arg(arg!(--cached))
        )
    )
    .subcommand(
        Command::new("tree")
        .about("Prints the project tree")
    )
    .subcommand(
        Command::new("build")
        .about("Builds the bulgogi project")
    )
    .subcommand(
        Command::new("clean")
        .about("Cleans the local project build files and cache")
    )
    .subcommand(
        Command::new("test")
        .about("Performs test functions -- used for development.")
    )
}

pub enum InfoKind {
    InitSuccess,
    SpawnSuccess,
    AddModuleSuccess,
    NoChange,
    DuplicateModule,
}

pub fn info(msg: InfoKind) {
    print!("[i] ");
    match msg {
        InfoKind::InitSuccess => println!("Successfully initialized project."),
        InfoKind::SpawnSuccess => println!("Successfully spawned project directories."),
        InfoKind::AddModuleSuccess => println!("Successfully added module to project."),
        InfoKind::NoChange => println!("No changes were made to the project state (project.yaml)"),
        InfoKind::DuplicateModule => println!("Duplicate module detected."),
    }
}

pub enum HelpKind {
    CyclicDependency,
    ProjectFound,
    NotInitialized,
    InitRequired,
    TargetNotFound,
    NoModuleDir,
    MissingSubdirs,
    ProjectInitFailed,
}

pub fn help(msg: HelpKind) {
    print!("[?] ");
    match msg {
        HelpKind::CyclicDependency => println!("A cyclic dependency in your project.yaml file was detected. Consider fixing this and trying again."),
        HelpKind::ProjectFound => println!("Found project.yaml -- no need to initialize."),
        HelpKind::NotInitialized => println!("Cannot find a project.yaml in the current directory."),
        HelpKind::InitRequired => println!("An initialized project is required to continue."),
        HelpKind::TargetNotFound => println!("Could not find specified or default target in project."),
        HelpKind::NoModuleDir => println!("Could not find the specified module directory. Try using the --create flag to automatically populate the module directories."),
        HelpKind::MissingSubdirs => println!("One or more of the module subdirs (/src, /inc, /src/inc) were not found."),
        HelpKind::ProjectInitFailed => println!("Failed to initialize project -- could not create project.yaml"),
    }
}

pub enum Prompt {
    AutoInit,
    AutoAddTarget,
    AutoAddSubdirs,
}

pub fn get_prompt(prompt: Prompt) -> String {
    match prompt {
        Prompt::AutoInit => String::from("Automatically initialize project in current directory?"),
        Prompt::AutoAddTarget => String::from("Automatically add the default target to the project?"),
        Prompt::AutoAddSubdirs => String::from("Automatically create missing subdirs for the module?"),
    }
}
