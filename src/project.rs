// Project module 

use std::collections::HashMap;

use serde_yaml::{Mapping, Value};

pub type TargetID = String;
pub type ModuleID = String;
pub type ProjectLayout = HashMap<TargetID, Vec<String>>;

pub enum Target{
    Executable,
    Library,
}

pub enum Module {
    Normal,
    Executable,
}

pub enum Dependency {
    Module(ModuleID),
    Target(TargetID),
}

pub struct Project {
    pub targets: HashMap<TargetID, Target>,
    pub modules: HashMap<ModuleID, Module>,
    pub deps: HashMap<TargetID, Dependency>,

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

        let targets = map.keys().filter_map(|k| {
            match k {
                Value::String(s) => Some(s.clone()),
                _ => None,
            }
        }).collect::<Vec<TargetID>>();

        // Collect values first as flat map then filter map against matches found in target list
        let modules = map.values()
            .filter_map(|v| match v { Value::Sequence(l) => Some(l), _ => None })
            .flat_map(|l| l.iter())
            .filter_map(|
            
        });

        // TODO: Find out in what order to load the mapping into the project.
        //       Maybe loading the deps first can allow the rest to be derived from there?

        project
    }
}
