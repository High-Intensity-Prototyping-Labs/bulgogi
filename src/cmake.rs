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
    pub links: Vec<CMakeTarget>,
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
        // Collect list of submodules
        let submodules = project.modules.keys().collect::<Vec<Submodule>>();
        
        // Glob lists before sorting by submodule
        let cmake_lists = project.deps.iter()
            .filter_map(|(target_id, dep_list)| filter_match!(project.targets.get_key_value(target_id), Some(entry), Some((entry, dep_list))))
            .map(|(entry, dep_list)| {
                CMakeList {
                    target: CMakeTarget::from((entry.0.clone(), entry.1.clone())),
                    links: dep_list.iter().map(|d| CMakeTarget::from(d.clone())).collect(),
                }
            }).collect::<Vec<CMakeList>>()
            .extend(submodules.iter().map(|s| {
                CMakeList {
                    target: CMakeTarget::from(s.clone()),
                    links: Vec::new(),
                }
            }).collect::<Vec<CMakeList>>());

        // let lists = cmake_lists.into_iter()
        //     .map(|l| )
    }
}

// TODO: Prepend CMakeTarget internal representation of target names with "lib" when appropriate.
impl From<(TargetID, Target)> for CMakeTarget {
    fn from((target_id, target): (TargetID, Target)) -> Self {
        match target {
            Target::Library => CMakeTarget::Library(target_id),
            Target::Executable => CMakeTarget::Executable(target_id),
        }
    }
}

// TODO: Prepend CMakeTarget internal representation of target names with "lib" when appropriate.
impl From<Dependency> for CMakeTarget {
    fn from(dep: Dependency) -> Self {
        match dep {
            Dependency::Module(m) => CMakeTarget::Library(m),
            Dependency::Target(t) => CMakeTarget::Library(t),
        }
    }
}

// TODO: Prepend CMakeTarget internal representation of target names with "lib" when appropriate.
impl From<Submodule> for CMakeTarget {
    fn from(submodule: Submodule) -> Self {
        CMakeTarget::Library(submodule)
    }
}
