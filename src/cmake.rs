// CMake Module
use crate::filter_match;
use crate::project::{Project, TargetID, Target, Dependency, Module, ModuleID, ProjectChild};

use std::collections::HashMap;

use itertools::Itertools;

pub type Submodule = String;
pub type CMakeList = Vec<CMakeTarget>;

#[derive(Debug)]
pub struct CMakeProject {
    pub submodules: Vec<Submodule>,
    pub lists: HashMap<Submodule, CMakeList>,
    pub targets: HashMap<Submodule, CMakeTarget>,
}


#[derive(Debug, Clone)]
pub enum CMakeTarget {
    Library(String),
    Executable(String),
}

impl CMakeTarget {
    pub fn is_exe(&self) -> bool {
        match self {
            CMakeTarget::Executable(_) => true,
            CMakeTarget::Library(_) => false,
        }
    }
}

impl CMakeProject {
    pub fn new() -> Self {
        CMakeProject {
            submodules: Vec::new(),
            lists: HashMap::new(),
            targets: HashMap::new(),
        }
    }
}

impl From<Project> for CMakeProject {
    fn from(project: Project) -> Self {
        let submodules = project.modules.keys().cloned().collect_vec();

        let exe_modules = project.deps.iter()
            .filter_map(|(tid, dep_list)| filter_match!(project.targets.get(tid), Some(t), Some((t, dep_list))))
            .filter_map(|(t, dep_list)| filter_match!(t, Target::Executable, Some(dep_list)))
            .flat_map(|dep_list| dep_list.iter())
            .filter_map(|d| filter_match!(d, Dependency::Module(m), Some(m)))
            .filter_map(|m| filter_match!(project.modules.get(m), Some(Module::Executable), Some(m.clone())))
            .collect_vec();

        dbg!(submodules);
        dbg!(exe_modules);

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
