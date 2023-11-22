// Project module
use crate::target::Target;
use crate::target::TARGET_DEFAULT;
use crate::dependency::{Dependency, DepKind};
use crate::client;
use crate::client::{InfoKind, HelpKind};

use std::fs;
use std::fs::File;
use std::path::Path;

use std::io;

const PROJECT_YAML: &str = "project.yaml";
const SRC_DIR: &str = "src";
const INC_DIR: &str = "inc";

pub struct Project {
    targets: Vec<Target>,
}

impl Project {
    pub fn new() -> Self {
        Project {
            targets: vec![],
        }
    }

    /// Initializes a bulgogi project in the PWD of the shell.
    pub fn init() {
        // Check if project.yaml exists 
        if Path::new(PROJECT_YAML).exists() {
            // Advise that project exists
            client::help(client::HelpKind::ProjectFound);
        } else {
            // Create project file with nothing in it
            if let Ok(_) = File::create(PROJECT_YAML) {
                // Project initialized
                client::info(InfoKind::InitSuccess);
            } else {
                // Could not create project.yaml 
                client::help(HelpKind::ProjectInitFailed);
            }
        }
    }

    /// Adds a module to the default target of the project.
    /// If no default target is found, it is created.
    pub fn add_module(&mut self, module_name: String) -> Result<(), io::Error> {
        // Fetch default target from project
        if let Some(target) = self.find_default_target_mut() {
        // Default target exists -- simply add module to it 
            target.deps.push((module_name.clone(), DepKind::Module));
        } else {
        // Default module not yet created -- make it
            self.targets.push(Target { deps: vec![(module_name.clone(), DepKind::Module)], ..Default::default() });
        }

        // Spawn (create) module directory
        Project::spawn_module(module_name)?;

        Ok(())
    }

    /// Adds a target to the project 
    pub fn add_target(&mut self, target_name: String) {
        self.targets.push(Target::from(target_name));
    }

    /// Returns a ref to a target which matches the criteria
    pub fn find_target(&self, name: &String) -> Option<&Target> {
        for target in &self.targets {
            if &target.name == name {
                return Some(target);
            }
        }
        None
    }

    /// Returns a mut ref to a target with `name`
    pub fn find_target_mut(&mut self, name: &String) -> Option<&mut Target> {
        for target in &mut self.targets {
            if &target.name == name {
                return Some(target);
            }
        }
        None
    }

    /// Returns a ref to the default target in the project 
    pub fn find_default_target(&self) -> Option<&Target> {
        return self.find_target(&Target::default().name);
    }

    /// Returns a mut ref to the default target in the project 
    pub fn find_default_target_mut(&mut self) -> Option<&mut Target> {
        return self.find_target_mut(&Target::default().name);
    }

    /// Saves the project to disk 
    pub fn save(&self) -> () {
        if let Ok(f) = File::options().write(true).open(PROJECT_YAML) {
        // Project file exists -- overwrite previous config
        } else {
        // Project file does not exist
        }
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
}
