// CMake Module
use crate::filter_match;
use crate::project::{Project, TargetID, Target, Dependency, Module, ModuleID, ProjectChild, ExecutableIter};

use std::collections::HashMap;

use itertools::Itertools;

pub type Submodule = String;

#[derive(Debug)]
pub struct CMakeProject {
    pub submodules: Vec<Submodule>,
    pub lists: HashMap<Submodule, CMakeList>,
}

#[derive(Debug)]
pub struct CMakeList {
    pub target: CMakeTarget,
    pub links: Vec<CMakeTarget>,
}

#[derive(Debug, Clone)]
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
        let submodules = project.modules.keys().cloned().collect_vec();

        // Names of executable modules
        let exemodules = submodules.iter()
            .filter_map(|sm| filter_match!(project.modules.get(sm), Some(Module::Executable), Some(sm)))
            .collect_vec();

        // Names of library modules
        let libmodules = submodules.iter()
            .filter(|sm| !exemodules.iter().any(|e| sm == e))
            .collect_vec();

        // Names of library targets
        let libtargets = project.targets.iter()
            .filter_map(|(tid, t)| filter_match!(t, Target::Library, Some(tid)))
            .collect_vec();

        // Names of executable targets
        let exetargets = project.targets.iter()
            .filter_map(|(tid, t)| filter_match!(t, Target::Executable, Some(tid)))
            .collect_vec();

        // Collection of executable modules, library modules and library targets into CMakeLists.
        let lists = exetargets.iter()
            .filter_map(|&e| filter_match!(project.deps.get(e), Some(dep_list), Some((e, dep_list))))
            .map(|(e, dep_list)| { 
                CMakeList {
                    target: CMakeTarget::Executable(e.to_string()),
                    links: dep_list.iter()
                            .filter(|d| !exemodules.iter().any(|e| d == e))
                            .cloned()
                            .map_into()
                            .collect_vec(),
                }
            })
            .collect_vec();

        dbg!(exemodules.clone());
        dbg!(libmodules);
        dbg!(exetargets.clone());
        dbg!(lists);

        // let lists = executables.iter()
        //    .filter_map(|e| filter_match!(project.deps.get(e), Some(dep_list), Some((e, dep_list))))

        todo!("Finish collecting the static lists of targets into CMakeLists.");

        CMakeProject::new()
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

impl PartialEq<String> for CMakeTarget {
    fn eq(&self, other: &String) -> bool {
        match self {
            CMakeTarget::Library(l) => l == other,
            CMakeTarget::Executable(x) => x == other,
        }
    }
}

impl PartialEq<&String> for CMakeTarget {
    fn eq(&self, other: &&String) -> bool {
        match self {
            CMakeTarget::Library(l) => &l == other,
            CMakeTarget::Executable(x) => &x == other,
        }
    }
}
