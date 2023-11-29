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
use std::process::Command;

use serde_yaml::{Value, Mapping, Sequence};

const PROJECT_YAML: &str = "project.yaml";
const SRC_DIR: &str = "src";
const INC_DIR: &str = "inc";

#[derive(Debug, Clone)]
pub struct Project {
    pub targets: Vec<Target>,
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
        if let Some(target) = self.targets.iter_mut().find(|t| t.name == target_name) {
            target.deps.push((module_name, DepKind::Module));
        } else {
            self.targets.push(Target::from((target_name, module_name)));
        }
    }

    /// Adds a target to the project 
    pub fn add_target(&mut self, target_name: String) {
        self.targets.push(Target::from(target_name));
    }

    /// Checks whether project has module 
    pub fn has_module(&self, module_name: &String) -> bool {
        for target in &self.targets {
            for dep in &target.deps {
                if let (m, DepKind::Module) = dep {
                    if m == module_name {
                        return true;
                    }
                }
            }
        }
        false
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

    /// Removes a module from the project
    pub fn rm_module(&mut self, target_name: String, module_name: String) {
        if let Some(target) = self.find_target_mut(&target_name) {
            let mut found = false;
            let new_deps: Vec<Dependency> = target.deps.clone().into_iter().filter(|d| {
                d.0 != module_name && d.1 == DepKind::Module
            }).collect();
        
            // TODO: Complete this part where if no modules were removed the user gets a 
            // message saying that the module could not be found. This is to present false
            // positives when deleting modules (typo or otherwise).
            //
            // Default behaviour should stay to keep a successful remove operation as silent


            target.deps = new_deps;
        }
    }

    /// Saves the project to disk 
    pub fn save(&self) -> Result<(), serde_yaml::Error> {
        let filtered_targets: Vec<Target> = self.targets.clone().into_iter().filter(|t| !t.deps.is_empty()).collect();
        let filtered_project = Project { targets: filtered_targets };

        if let Ok(f) = File::options().write(true).create(true).truncate(true).open(PROJECT_YAML) {
            serde_yaml::to_writer(f, &Mapping::from(filtered_project))?;
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

    /// Outputs a tree diagram using `tree` 
    pub fn tree(&self) {
        let mut cmd = Command::new("tree");
        for target in &self.targets {
            for dep in &target.deps {
                if let (module, DepKind::Module) = dep {
                    cmd.arg(module);
                }
            }
        }
        let tree = cmd.output().expect("tree command failed").stdout;
        println!("{}", String::from_utf8(tree).expect("UTF-8 tree to string failed"));
    }
}

impl From<Mapping> for Project {
    fn from(mapping: Mapping) -> Self {
        // Create blank project 
        let mut project = Project::new();

        // Parse mapping into targets, add to project
        let ref_mapping = mapping.clone();
        for (key, value) in mapping {
            // Only consider String keys and Sequence values
            if let (Value::String(target), Value::Sequence(sequence)) = (key, value) {
                // Create a target with empty deps based on key string
                let mut new_target = Target::from(target);

                // For each entry in the sequence
                for entry in sequence {
                    // Only consider strings in the sequence
                    if let Value::String(dep_string) = entry {
                        if ref_mapping.keys().any(|k| matches!(k, Value::String(t) if t == &dep_string)) {
                        // A matching dependency name has been found in the list of targets (keys)
                            new_target.deps.push((dep_string, DepKind::Target));
                        } else {
                        // No matching dependency names in the target list
                            new_target.deps.push((dep_string, DepKind::Module));
                        }
                    }
                }
                project.targets.push(new_target);
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
