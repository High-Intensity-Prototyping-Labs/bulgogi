// Project module
use crate::target::Target;
use crate::target::TARGET_DEFAULT;
use crate::dependency::{Dependency, DepKind};
use crate::client;
use crate::client::{InfoKind, HelpKind};

use std::io;
use std::fs;
use std::fs::File;
use std::path::Path;

use serde_yaml::{Value, Mapping, Sequence};

const PROJECT_YAML: &str = "project.yaml";
const SRC_DIR: &str = "src";
const INC_DIR: &str = "inc";

#[derive(Debug, Clone)]
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
    pub fn add_module(&mut self, target_name: String, module_name: String) {
        // Fetch target from project
        if let Some(target) = self.find_target_mut(&target_name) {
        // Target exists -- simply add module to it 
            target.deps.push((module_name.clone(), DepKind::Module));
        } else {
        // Target not yet created -- make it
            self.targets.push(Target::from((target_name, module_name.clone())));
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

    /// Loads project from disk 
    pub fn load() -> Result<Self, io::Error> {
        match File::open(PROJECT_YAML) {
            Ok(f) => {
            // Project file exists -- load it
                let yaml = serde_yaml::from_reader::<File, Mapping>(f).or_else(|_| {
                    client::help(HelpKind::YamlParsingError);
                    Err(Mapping::new())
                });

                let mapping = yaml.unwrap(); // panic here if YAML failed to parse
                let project = Project::from(mapping);

                Ok(project)
            }
            Err(e) => {
            // Project not found or initialized -- notify 
                client::help(HelpKind::ProjectLoadFailed);
                Err(e)
            }
        }
    }

    /// Saves the project to disk 
    pub fn save(&self) -> Result<(), serde_yaml::Error> {
        if let Ok(f) = File::options().write(true).create(true).open(PROJECT_YAML) {
            serde_yaml::to_writer(f, &Mapping::from(self.clone()))?;
        }
        Ok(())
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

impl From<Mapping> for Project {
    fn from(mapping: Mapping) -> Self {
        // Create blank project 
        let mut project = Project::new();

        // Parse mapping into targets, add to project
        for (key, value) in mapping {
            // Only consider String keys and Sequence values
            if let (Value::String(target), Value::Sequence(sequence)) = (key, value) {
                // Create a target with empty deps based on key string
                let mut new_target = Target::from(target);

                // For each entry in the sequence
                for entry in sequence {
                    // Only consider strings in the sequence
                    if let Value::String(module) = entry {
                        // If it's a string, it's a module
                        new_target.deps.push((module, DepKind::Module));
                    }
                }
                project.targets.push(new_target);
            }
        }

        // Infer which dependencies are actually targets
        let project_ref = project.clone();
        for target in &mut project.targets {
            for dep in &mut target.deps {
                if let Some(_) = project_ref.find_target(&dep.0) {
                // Target with matching name found -- therefore target
                    dep.1 = DepKind::Target;
                }
            }
        }
        project
    }
}

impl From<Project> for Mapping {
    fn from(project: Project) -> Self {
        let mut mapping = Mapping::new();

        for target in project.targets {
            let mut sequence = Sequence::new();

            for dep in target.deps {
                sequence.push(Value::String(dep.0));
            }

            mapping.insert(Value::String(target.name), Value::Sequence(sequence));
        }
        mapping
    }
}
