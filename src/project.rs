// Project module 

use std::collections::HashMap;

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
pub type ProjectLayout = HashMap<TargetID, Vec<String>>;

#[derive(Debug)]
pub enum Target{
    Executable,
    Library,
}

#[derive(Debug)]
pub enum Module {
    Normal,
    Executable,
}

#[derive(Debug)]
pub enum Dependency {
    Module(ModuleID),
    Target(TargetID),
}

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
        let mut project = Project::new();

        // Get list of targets
        let targets = map.keys()
            .filter_map(|k| filter_match!(k, Value::String(s), Some(s.clone())))
            .collect::<Vec<TargetID>>();

        // Collect values first as flat map then filter map against matches found in target list
        let mut modules = map.values()
            .filter_map(|v| filter_match!(v, Value::Sequence(seq), Some(seq)))
            .flat_map(|seq| seq.iter().filter_map(|entry| filter_match!(entry, Value::String(s), Some(s.clone()))))
            .filter(|s| !targets.iter().any(|t| t == s))
            .collect::<Vec<ModuleID>>();
        modules.dedup();

        // Get dependencies
        let deps = map.iter()
            .filter_map(|(k, v)| filter_match!((k, v), (Value::String(target_id), Value::Sequence(seq)), Some((target_id.clone(), seq))))
            .map(|(target_id, seq)| (target_id, seq.iter().filter_map(|entry| filter_match!(entry, Value::String(dep_str), Some(dep_str.clone())))))
            .map(|(target_id, dep_strs)| (target_id, dep_strs.map(|d| {
                if targets.iter().any(|t| t == &d) { 
                    Dependency::Target(d) 
                } else if modules.iter().any(|m| m == &d) { 
                    Dependency::Module(d) 
                } else {
                    panic!("Could not find dependency {} in targets or modules list. This should not be possible given that all entries in the project.yaml are added to one or the other", d);
                }
            }).collect::<Vec<Dependency>>()))
            .collect::<HashMap<TargetID, Vec<Dependency>>>();

        dbg!(map);
        dbg!(targets);
        dbg!(modules);
        dbg!(deps);

        project
    }
}
