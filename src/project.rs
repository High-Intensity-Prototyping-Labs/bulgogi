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
        let modules = map.values()
            .filter_map(|v| match v { Value::Sequence(seq) => Some(seq), _ => None })
            .flat_map(|seq| seq.iter().filter_map(|v| match v { Value::String(s) => Some(s.clone()), _ => None }))
            .filter(|s| !targets.iter().any(|t| t == s))
            .map(|s| match s.contains("*") { true => (s, Module::Executable), false => (s, Module::Normal) })
            .collect::<HashMap<ModuleID, Module>>();

        // Get dependencies
        let deps = map.iter()
            .filter_map(|(k, v)| match (k, v) { (Value::String(target_id), Value::Sequence(seq)) => Some((target_id.clone(), seq)), _ => None })
            .map(|(target_id, seq)| (target_id, seq.iter().filter_map(|v| match v { Value::String(dep_str) => Some(dep_str.clone()), _ => None }).collect::<Vec<String>>()))
            .map(|(target_id, dep_list)| (target_id, dep_list.iter().map(|d| match targets.iter().any(|t| t == d) { true => Dependency::Target(d.clone()), false => Dependency::Module(d.clone()) }).collect::<Vec<Dependency>>() ))
            .collect::<HashMap<TargetID, Vec<Dependency>>>();

        dbg!(map);
        dbg!(targets);
        dbg!(modules);
        dbg!(deps);

        project
    }
}
