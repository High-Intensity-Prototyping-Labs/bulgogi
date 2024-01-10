// CMake Module
use crate::filter_match;
use crate::project::{Project, TargetID, Target, Dependency, Module, ModuleID, ProjectChild, ExecutableIter};

use std::collections::HashMap;

use itertools::Itertools;

pub type Submodule = String;
pub type LibName = String;
pub type ExeName = String;

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
    Library(LibName),
    Executable(Submodule, ExeName),
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

        // Group exemodules and exetargets together (proxies)
        let exegroups = exetargets.iter()
            .filter_map(|&et| filter_match!(project.deps.get(et), Some(et_deps), Some((et, et_deps))))
            .filter_map(|(et, et_deps)| {
                match et_deps.iter().find(|&d| exemodules.iter().any(|&em| d == em)) {
                    Some(et_dep_exe) => Some((et, et_dep_exe)),
                    _ => None,
                }
            })
            .map(|(et, et_dep_exe)| (et.to_string(), et_dep_exe.to_string()))
            .collect_vec();

        // Collection of executable modules
        let exelists = exegroups.iter()
            .filter_map(|(tar, sub)| filter_match!(project.deps.get(tar), Some(dep_list), Some((tar, sub, dep_list))))
            .map(|(tar, sub, dep_list)| { 
                CMakeList {
                    target: CMakeTarget::Executable(sub.clone(), tar.clone()),
                    links: dep_list.iter()
                            .filter(|d| !exemodules.iter().any(|e| d == e))
                            .cloned()
                            .map_into()
                            .collect_vec(),
                }
            })
            .collect_vec();

        // Collection of modules compiled as libraries
        let libmodulelists = libmodules.iter()
            .map(|lm| {
                CMakeList {
                    target: CMakeTarget::Library(lm.to_string()),
                    links: Vec::new(),
                }
            })
            .collect_vec();

        // Shorthand for library targets with dependencies (go in top-level CMakeList for now)
        let toplist = libtargets.iter()
            .filter_map(|&lt| filter_match!(project.deps.get(lt), Some(dep_list), Some((lt, dep_list))))
            .map(|(lt, dep_list)| {
                CMakeList {
                    target: CMakeTarget::Library(lt.to_string()),
                    links: dep_list.iter()
                            .cloned()
                            .map_into()
                            .collect_vec(),
                }
            })
            .collect_vec();

        // Associate lists to submodules
        //let exesubmodules = exemodules.iter()
        //    .map(|e| )

        dbg!(submodules.clone());
        dbg!(exemodules.clone());
        dbg!(libmodules);
        dbg!(exetargets.clone());
        dbg!(exelists);
        dbg!(exegroups);
        dbg!(libmodulelists);
        dbg!(toplist);

        // let lists = executables.iter()
        //    .filter_map(|e| filter_match!(project.deps.get(e), Some(dep_list), Some((e, dep_list))))

        todo!("Finish collecting the static lists of targets into CMakeLists.");

        CMakeProject::new()
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
            CMakeTarget::Executable(_, x) => x == other,
        }
    }
}

impl PartialEq<&String> for CMakeTarget {
    fn eq(&self, other: &&String) -> bool {
        match self {
            CMakeTarget::Library(l) => &l == other,
            CMakeTarget::Executable(_, x) => &x == other,
        }
    }
}
