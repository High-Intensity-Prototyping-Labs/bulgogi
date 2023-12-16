// Project module
use crate::target::Target;
use crate::dependency::{Dependency, DepKind, DepFlag};

use serde_yaml::{Value, Mapping, Sequence};

#[derive(Debug, Clone)]
pub struct Project {
    pub targets: Vec<Target>,
}

impl Project {
    pub fn new() -> Self {
        Project {
            targets: Vec::new(),
        }
    }

    /// Returns whether the project has no targets (empty)
    pub fn empty(&self) -> bool {
        self.targets.is_empty()
    }

    /// Returns an iterator over all the module-type dependencies of the project
    pub fn modules(self) -> impl Iterator<Item=Dependency> {
        self.targets.into_iter().flat_map(|t| t.deps.into_iter().filter(|d| matches!(d.kind, DepKind::Module)))
    }

    /// Returns an iterator over all project targets 
    pub fn targets(self) -> impl Iterator<Item=Target> {
        self.targets.into_iter()
    }

    /// Returns a mutable iterator over all project targets
    pub fn targets_mut(&mut self) -> std::slice::IterMut<'_, Target> {
        self.targets.iter_mut()
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
                            new_target.deps.push(Dependency::from((dep_string, DepKind::Target)));
                        } else {
                        // No matching dependency names in the target list
                            if dep_string.contains("*") {
                            // The project configuration indicates the module contains a main()
                            // routine
                                new_target.deps.push(Dependency::from((dep_string.replace("*", ""), DepKind::Module, DepFlag::Executable)));
                            } else {
                            // This module contains no main() routine
                                new_target.deps.push(Dependency::from((dep_string, DepKind::Module)));
                            }
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
                match dep.flag {
                    DepFlag::Executable => sequence.push(Value::String(format!("{}*", dep.name))),
                    _ => sequence.push(Value::String(dep.name)),
                }
            }

            mapping.insert(Value::String(target.name), Value::Sequence(sequence));
        }
        mapping
    }
}
