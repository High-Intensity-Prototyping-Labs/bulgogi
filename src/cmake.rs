// CMake Module
use crate::filter_match;
use crate::project::{Project, TargetID, Target, Dependency, Module, ModuleID, ProjectChild};

use std::collections::HashMap;

pub type Submodule = String;

pub struct CMakeProject {
    pub submodules: Vec<Submodule>,
    pub lists: HashMap<Submodule, CMakeList>,
}

pub struct CMakeList {
    pub target: CMakeTarget,
    pub links: Option<Vec<CMakeTarget>>,
}

pub enum CMakeTarget {
    Library(String),
    Executable(String),
}

impl CMakeProject {
    pub fn new() -> Self {
        CMakeProject {
            submodules: Vec::new(),
            lists: HashMap::new(),
        }
    }
}

impl From<Project> for CMakeProject {
    fn from(project: Project) -> Self {
        let submodules = project.modules.keys().collect();
        let lists = project.deps.iter()
            .filter_map(|(target_id, dep_list)| filter_match!(project.targets.get_key_value(target_id), Some(entry), Some((entry, dep_list))))
            .map(|(entry, dep_list)| {
                CMakeList {
                    target: CMakeTarget::from((entry.0.clone(), entry.1.clone())),
                    links: ,
                }
            })
    }
}

impl From<(TargetID, Target)> for CMakeTarget {
    fn from((target_id, target): (TargetID, Target)) -> Self {
        match target {
            Target::Library => CMakeTarget::Library(target_id),
            Target::Executable => CMakeTarget::Executable(target_id),
        }
    }
}

impl From<Dependency> for CMakeTarget {
    fn from(dep: Dependency) -> Self {
        match dep {
            Dependency::Module(m) => CMakeTarget::Library(m),
            Dependency::Target(t) => CMakeTarget::Library(t),
        }
    }
}
