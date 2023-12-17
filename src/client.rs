// Module dedicated to the cli.
use crate::template;
// use crate::target::Target;
use crate::project::{Project, Module, Dependency};
// use crate::dependency::{Dependency, DepKind};

use std::io;
use std::process;
use std::fs::File;
use std::path::Path;

use std::fs;
use clap::{arg, Command};
use serde_yaml::Mapping;

/// Relative path to the project config file
const PROJECT_YAML: &str = "project.yaml";
/// Top-level module source directory
const SRC_DIR: &str = "src";
/// Top-level module include directory
const INC_DIR: &str = "inc";

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
            init();
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

/// Initializes the project configuration file in the current directory.
pub fn init() {
    // Check if project.yaml exists 
    if Path::new(PROJECT_YAML).exists() {
        // Advise that project exists
        help(HelpKind::ProjectFound);
    } else {
        // Create project file with nothing in it
        if let Ok(_) = File::create(PROJECT_YAML) {
            // Do nothing
        } else {
            // Could not create project.yaml 
            help(HelpKind::ProjectInitFailed);
        }
    }
}

/// High-level cli func to add a module to the project in the PWD.
pub fn add_module(module: impl Into<String>, target: impl Into<String>) -> Result<(), io::Error> {
    // Collect args
    let module = module.into();
    let target = target.into();

    // Load Project
    let mut project = load()?;
    
    // Add module to modules map if missing
    if project.modules.contains_key(&module) == false {
        // Create clean copy of module ID for internal representation
        let module_clean = module.replace("*", "");

        // Account for executable indicator
        if module.contains("*") {
            project.modules.insert(module_clean, Module::Executable);
        } else {
            project.modules.insert(module_clean, Module::Normal);
        }
    }

    // Add module as dependency to target
    if let Some(target_deps) = project.deps.get_mut(&target) {
        // Check for duplicate module-dependency for target
        if target_deps.contains(&Dependency::Module(module.clone())) {
            info(InfoKind::DuplicateModule);
        } else {
            target_deps.push(Dependency::Module(module));
        }
    } else {
        // Target not found in project dep tree
        help(HelpKind::TargetNotFound);
    }

    // Save project
    

    Ok(())
}

/// High-level func which removes a module from the project
pub fn rm_module(target_name: String, module_name: String, cached: bool) {
}

/// Spawns a module directory with required subdirs
pub fn spawn_module(module_name: String) -> Result<(), io::Error> {
    let path = Path::new(&module_name);
    let src = path.join(SRC_DIR);
    let inc = path.join(INC_DIR);
    let pinc = src.join(SRC_DIR);

    if !path.exists() {
    // Module dir missing -- create 
        fs::create_dir_all(src)?;
        fs::create_dir_all(inc)?;
        fs::create_dir_all(pinc)?;
    }

    Ok(())
}


/// Loads project from the current working directory
pub fn load() -> Result<Project, io::Error> {
    match File::open(PROJECT_YAML) {
        Ok(f) => {
        // Project file exists -- load it
            let yaml = serde_yaml::from_reader::<File, Mapping>(f).or_else(|_| {
                help(HelpKind::YamlParsingError);
                Err(Mapping::new())
            });

            let mapping = yaml.unwrap(); // panic here if YAML failed to parse
            let project = Project::from(mapping);

            Ok(project)
        }
        Err(e) => {
        // Project not found or initialized -- notify 
            help(HelpKind::ProjectLoadFailed);
            Err(e)
        }
    }
}

/// Saves the project to disk 
pub fn save(project: &Project) -> Result<(), serde_yaml::Error> {
    Ok(())
}


/// Displays a tree diagram of the project modules 
pub fn tree() {
    // Load project 
    match load() {
        Ok(project) => {
            // Declare tree command process
            let mut cmd = process::Command::new("tree");

            // Add each module-type dependency (directories) to tree command args
            cmd.args(project.clone().modules.keys());

            // Match result of running command
            match cmd.output() {
                Ok(c) => {
                    match String::from_utf8(c.stdout) {
                        Ok(_) if project.modules.is_empty() => println!(""),
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

/// Loads a template from the templates dir 
pub fn template() -> Result<(), io::Error> {
    template::load().expect("error");

    println!("::TESTING::");

    let project = load()?;
    for target in project.targets {

    }

    if let Ok(project) = load() {
        println!("!!Printing modules!!");
        for module in project.modules.keys() {
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
