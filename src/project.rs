// Project module 

use std::collections::HashMap;

use itertools::Itertools;
use serde_yaml::{Mapping, Value};

macro_rules! filter_match {
    ($what:expr, $pat:pat, $ret:expr) => {
        match $what {
            $pat => $ret,
            _ => None,
        }
    }
}

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
