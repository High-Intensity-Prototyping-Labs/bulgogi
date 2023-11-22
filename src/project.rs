// Project module
use crate::target::Target;
use crate::target::TARGET_DEFAULT;
use crate::dependency::{Dependency, DepKind};
use crate::client;
use crate::client::{InfoKind, HelpKind};

use std::fs::File;
use std::path::Path;

const PROJECT_YAML: &str = "project.yaml";

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

    /// Adds a module to the project 
    ///
    /// TODO: Here's the idea: the struct-level API should reflect the terminal.
    ///         The project-level '.add_module' should simply add a module to the default target.
    ///         If a specific target is needed, the target should first be .found() and *then* have 
    ///         a module added. Otherwise, we are simply not respecting the object-model and doing
    ///         a bunch of redundant searching and if-checking.
    ///
    ///     So the premise goes something like this: project.add_module() => adds module to default
    ///                                             project.add_target() => adds empty target 
    ///                                             target.add_module() => adds a module to this
    pub fn add_module(&mut self, module_name: String) {
        if let Some(target) = self.find_default_target_mut() {
        // Default target exists -- simply add module to it 
            target.deps.push((module_name, DepKind::Module));
        } else {
        // Default module not yet created -- make it
            self.targets.push(Target { deps: vec![(module_name, DepKind::Module)], ..Default::default() });
        }
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
}
