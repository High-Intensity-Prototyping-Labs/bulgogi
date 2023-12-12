// Module dedicated to the cli.
use std::io;
use std::process;
use clap::{arg, Command};
use crate::template;
use crate::target::Target;
use crate::project::Project;
use crate::cmake::CMakeList;
use crate::dependency::{Dependency, DepKind};

/// Shorthand to get an argument from a cli match
macro_rules! get_one {
    ($x:expr, $y:ty, $z:expr) => {
        $x.get_one::<$y>($z).expect("required").to_owned()
    };
}

/// Shorthand to get true/false state of flag from cli match
macro_rules! get_flag {
    ($x:expr, $y:expr) => {
        $x.get_flag($y)
    }
}


/// Handles the next CLI command and its arguments.
pub fn next_command() -> Result<(), io::Error> {
    let matches = cli().get_matches();

    match matches.subcommand() {
        Some(("init", _)) => {
            // Initialize project in the PWD
            Project::init();
        }
        Some(("module", cmd)) => {
            match cmd.subcommand() {
                Some(("add", sub)) => {
                    // Placehold add command arguments
                    let module = get_one!(sub, String, "MODULE");
                    let target = get_one!(sub, String, "TARGET");
                    add_module(target, module)?;
                }
                Some(("rm", sub)) => {
                    // Placehold add command arguments
                    let module = get_one!(sub, String, "MODULE");
                    let target = get_one!(sub, String, "TARGET");
                    let cached = get_flag!(sub, "cached");
                    rm_module(target, module, cached);
                }
                _ => unreachable!(),
            }
        }
        Some(("tree", _)) => {
            tree();
        }
        Some(("template", _)) => {
            template()?;
        }
        _ => unreachable!(),
    }

    Ok(())
}

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
        Command::new("module")
        .about("Manage project modules")
        .arg_required_else_help(true)
        .subcommand(
            Command::new("add")
            .about("Adds a new module to the bulgogi project")
            .arg(arg!(<MODULE> "Module name / directory"))
            .arg(
                arg!(<TARGET> "Target for which the module is a dependency")
                .default_value("default")
                .required(false)
            )
            .arg(arg!(--create))
        )
        .subcommand(
            Command::new("rm")
            .about("Removes a module from the bulgogi project")
            .alias("remove")
            .arg(arg!(<MODULE> "Module name / directory"))
            .arg(
                arg!(<TARGET> "Target associated to the module")
                .default_value("default")
                .required(false)
            )
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
        Command::new("template")
        .about("Tests the templating functionality")
    )
}

/// High-level cli func to add a module to the project in the PWD.
pub fn add_module(target: String, module: String) -> Result<(), io::Error> {
    // Load project
    match Project::load() {
        Ok(mut project) => {
            if project.has_module(&module) {
            // Duplicate found, notify
                info(InfoKind::DuplicateModule);
            } else {
            // No duplicates found, continue
                // Find matching target 
                if let Some(target) = project.targets_mut().find(|t| t.name == target) {
                    // Match found, add module 
                    target.deps.push(Dependency::from((module.clone(), DepKind::Module)));
                } else {
                    // No matching target found, create it 
                    project.targets.push(Target::from((target, module.clone())));
                }

                // Spawn module directory 
                Project::spawn_module(module)?;

                // Save project 
                project.save().expect("yaml");
            }
        }
        Err(e) => {
            if e.kind() == io::ErrorKind::NotFound {
                help(HelpKind::InitRequired);
            } else {
                println!("{}", e);
            }
        }
    }
    Ok(())
}

/// Displays a tree diagram of the project modules 
pub fn tree() {
    // Load project 
    match Project::load() {
        Ok(project) => {
            // Declare tree command process
            let mut cmd = process::Command::new("tree");

            // Add each module-type dependency (directories) to tree command args
            cmd.args(project.clone().modules().map(|m| m.name));

            // Match result of running command
            match cmd.output() {
                Ok(c) => {
                    match String::from_utf8(c.stdout) {
                        Ok(_) if project.empty() => println!(""),
                        Ok(tree) => println!("{}", tree),
                        Err(e) => println!("Failed to parse tree command output ({})", e),
                    }
                }
                Err(e) => println!("Failed to run tree command ({})", e)
            }
        }
        Err(e) => {
            if e.kind() == io::ErrorKind::NotFound {
                help(HelpKind::InitRequired);
            } else {
                println!("{}", e);
            }
        }
    }
}

/// Removes a module from the project 
pub fn rm_module(target: String, module: String, cached: bool) {
    // Load project
    if let Ok(mut project) = Project::load() {
        project.rm_module(target, module, cached);
        project.save().expect("yaml");
    } else {
        help(HelpKind::ProjectLoadFailed);
    }
}

/// Loads a template from the templates dir 
pub fn template() -> Result<(), io::Error> {
    template::load().expect("error");

    println!("::TESTING::");

    let project = Project::load()?;
    for target in project.targets {
        let _ = CMakeList::from(target);
    }

    if let Ok(project) = Project::load() {
        println!("!!Printing modules!!");
        for module in project.modules() {
            println!("{:#?}", module);
        }
    }

    Ok(())
}

pub enum InfoKind {
    InitSuccess,
    SpawnSuccess,
    AddModuleSuccess,
    NoChange,
    DuplicateModule,
    ModuleRemoved,
}

pub fn info(msg: InfoKind) {
    match msg {
        InfoKind::InitSuccess => println!("Successfully initialized project."),
        InfoKind::SpawnSuccess => println!("Successfully spawned project directories."),
        InfoKind::AddModuleSuccess => println!("Successfully added module to project."),
        InfoKind::NoChange => println!("No changes were made to the project state (project.yaml)"),
        InfoKind::DuplicateModule => println!("Duplicate module detected."),
        InfoKind::ModuleRemoved => println!("Successfully removed module."),
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
    YamlParsingError,
    ProjectLoadFailed,
}

pub fn help(msg: HelpKind) {
    match msg {
        HelpKind::CyclicDependency => println!("A cyclic dependency in your project.yaml file was detected. Consider fixing this and trying again."),
        HelpKind::ProjectFound => println!("Found project.yaml -- no need to initialize."),
        HelpKind::NotInitialized => println!("Cannot find a project.yaml in the current directory."),
        HelpKind::InitRequired => println!("An initialized project is required to continue."),
        HelpKind::TargetNotFound => println!("Could not find specified or default target in project."),
        HelpKind::NoModuleDir => println!("Could not find the specified module directory. Try using the --create flag to automatically populate the module directories."),
        HelpKind::MissingSubdirs => println!("One or more of the module subdirs (/src, /inc, /src/inc) were not found."),
        HelpKind::ProjectInitFailed => println!("Failed to initialize project -- could not create project.yaml"),
        HelpKind::YamlParsingError => println!("Failed to parse YAML into mapping -- check structure for corruption"),
        HelpKind::ProjectLoadFailed => println!("Failed to load project."),
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
