// Project module 
use crate::filter_match;

use std::collections::HashMap;

use itertools::Itertools;
use serde_yaml::{Mapping, Value};

pub type TargetID = String;
pub type ModuleID = String;

#[derive(Debug, Clone)]
pub enum Target{
    Executable,
    Library,
}

#[derive(Debug, Clone)]
pub enum Module {
    Normal,
    Executable,
}

#[derive(Debug, Clone, PartialEq)]
pub enum Dependency {
    Module(ModuleID),
    Target(TargetID),
}

#[derive(Debug, Clone)]
pub struct Project {
    pub targets: HashMap<TargetID, Target>,
    pub modules: HashMap<ModuleID, Module>,
    pub deps: HashMap<TargetID, Vec<Dependency>>,

}

impl Project {
    pub fn new() -> Self {
        Project {
            targets: HashMap::new(),
            modules: HashMap::new(),
            deps: HashMap::new(),
        }
    }

    /// Returns `true` if any targets in the project list `module` as one of its dependencies.
    /// Otherwise `false` implies that no targets depend on this module.
    pub fn any_depends(&self, module: &ModuleID) -> bool {
        self.deps.values().flatten().any(|d| d == module)
    }

    /// Returns an iterator over the project's executable TargetIDs
    pub fn executable_targets(&self) -> impl Iterator<Item=TargetID> + '_ {
        self.targets.iter().filter_map(|t| filter_match!(t.1, Target::Executable, Some(t.0.clone())))
    }

    /// Returns an iterator over the project targets
    pub fn targets(&self) -> impl Iterator<Item=(&TargetID, &Target)> {
        self.targets.iter()
    }

    // /// Returns a custom iterator over the project modules 
    // pub fn modules(&self) ->  {

    // }
}

/// Executable iterator struct
pub struct Executable<I, F> {
    iter: I,
    f: F,
}

/// Implement the Executable iterator
impl<I, F> Iterator for Executable<I, F> 
    where 
    I: Iterator,
    F: Fn(&I::Item) -> bool,
{
    type Item = I::Item;

    fn next(&mut self) -> Option<Self::Item> {
        match self.iter.next() {
            Some(e) if (self.f)(&e) => Some(e),
            _ => None,
        }
    }
}

/// Implement the Executable struct
impl<I, F> Executable<I, F> {
    pub fn new(iter: I, f: F) -> Executable<I, F> {
        Executable { iter, f }
    }
}

/// Extension trait for the Executable iterator 
pub trait ExecutableIter<F>: Sized {
    fn executable(self, f: F) -> Executable<Self, F> where Self: Sized;
}

/// Implement Executable extension trait for Iterator
impl<I, F> ExecutableIter<F> for I 
    where 
    I: Iterator + Sized,
    F: Fn(&I::Item) -> bool,
{
    fn executable(self, f: F) -> Executable<Self, F> where Self: Sized {
        Executable { iter: self, f }
    }
}


impl From<Mapping> for Project {
    fn from(map: Mapping) -> Self {
        // Get list of target_list
        let target_list = map.keys()
            .filter_map(|k| filter_match!(k, Value::String(s), Some(s.clone())))
            .collect::<Vec<TargetID>>();

        // TODO:
        // Detect bug with conflicting module indicators 

        // Collect values first as flat map then filter map against matches found in target list
        let modules = map.values()
            .filter_map(|v| filter_match!(v, Value::Sequence(seq), Some(seq)))
            .flat_map(|seq| seq.iter().filter_map(|entry| filter_match!(entry, Value::String(s), Some(s.clone()))))
            .filter(|s| !target_list.iter().any(|t| t == s))
            .dedup_by(|s1, s2| s1.replace("*", "") == s2.replace("*", ""))
            .map(|s| (s.clone(), s.contains("*").then_some(Module::Executable).unwrap_or(Module::Normal)))
            .map(|(s, m)| (s.replace("*", ""), m))
            .collect::<HashMap<ModuleID, Module>>();

        // Get dependencies
        let deps = map.iter()
            .filter_map(|(k, v)| filter_match!((k, v), (Value::String(target_id), Value::Sequence(seq)), Some((target_id.clone(), seq))))
            .map(|(target_id, seq)| (target_id, seq.iter().filter_map(|entry| filter_match!(entry, Value::String(dep_str), Some(dep_str.clone())))))
            .map(|(target_id, dep_strs)| (target_id, dep_strs.map(|s| s.replace("*", ""))))
            .map(|(target_id, dep_strs)| (target_id, dep_strs.map(|d| {
                if target_list.iter().any(|t| t == &d) { 
                    Dependency::Target(d) 
                } else if modules.keys().any(|m| m == &d) { 
                    Dependency::Module(d) 
                } else {
                    panic!("Could not find dependency {} in target or modules list. This should not be possible given that all entries in the project.yaml are added to one or the other", d);
                }
            }).collect::<Vec<Dependency>>()))
            .collect::<HashMap<TargetID, Vec<Dependency>>>();

        // HashMap the Targets 
        let targets = deps.iter()
            .map(|(t, d)| (t, d.iter().filter_map(|d| filter_match!(d, Dependency::Module(m), Some(m.clone())))))
            .map(|(t, m)| (t, m.filter_map(|m| filter_match!(modules.get(&m), Some(module), Some(module)))))
            .map(|(t, modules)| (t.clone(), modules.into_iter().any(|m| matches!(m, Module::Executable)).then_some(Target::Executable).unwrap_or(Target::Library)))
            .collect::<HashMap<TargetID, Target>>();

        // Return the initialized project
        Project {
            targets,
            modules,
            deps,
        }
    }
}

impl From<Project> for Mapping {
    fn from(project: Project) -> Self {
        project.deps.iter()
            .map(|(target_id, dep_list)| (target_id, dep_list.iter().map(|d| {
                match d {
                    Dependency::Module(m) => m,
                    Dependency::Target(t) => t,
                }
            })))
            .map(|(target_id, dep_strs)| (target_id, dep_strs.map(|d| Value::String(d.to_string())).collect::<Vec<Value>>()))
            .map(|(target_id, value_seq)| (Value::String(target_id.clone()), Value::Sequence(value_seq)))
            .collect::<Mapping>()
    }
}

impl From<Dependency> for String {
    fn from(dep: Dependency) -> Self {
        match dep {
            Dependency::Module(m) => m,
            Dependency::Target(t) => t,
        }
    }
}

impl PartialEq<String> for Dependency {
    fn eq(&self, other: &String) -> bool {
        match self {
            Dependency::Module(m) => m == other,
            Dependency::Target(t) => t == other,
        }
    }
}

impl ToString for Dependency {
    fn to_string(&self) -> String {
        match self {
            Dependency::Module(m) => m.clone(),
            Dependency::Target(t) => t.clone(),
        }
    }
}

pub trait ProjectChild {
    fn module_ids(&self) -> Vec<ModuleID>;
}

impl ProjectChild for Vec<Dependency> {
    fn module_ids(&self) -> Vec<ModuleID> {
        self.iter()
            .filter_map(|d| filter_match!(d, Dependency::Module(m), Some(m.clone())))
            .collect()
    }
}
