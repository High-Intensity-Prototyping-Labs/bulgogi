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

        let executables = project.modules.iter()
            .filter_map(|(mid, m)| filter_match!(m, Module::Executable, Some(mid)));

        // Closure which returns `Some()` Project.targets entries if it contains an 
        // executable component module or `None` otherwise.
        let executable = |entry: (&TargetID, &Vec<Dependency>)| {
            match entry.1.iter().find(|d| executables.clone().any(|m| d == &m)) {
                Some(_) => Some(entry),
                None => None,
            }
        };

        let proxies = project.deps.iter()
            .filter_map(executable)
            .collect_vec();

        dbg!(submodules);

        CMakeProject::new()
    }
}

// impl From<Project> for CMakeProject {
//     fn from(project: Project) -> Self {
//         // Collect list of submodules
//         let submodules = project.modules.keys().cloned().collect::<Vec<Submodule>>();
//         
//         // Glob lists before sorting by submodule
//         let cmake_lists = project.deps.iter()
//             .filter_map(|(target_id, dep_list)| filter_match!(project.targets.get_key_value(target_id), Some(entry), Some((entry, dep_list))))
//             .map(|(entry, dep_list)| {
//                 CMakeList {
//                     target: CMakeTarget::from((entry.0.clone(), entry.1.clone())),
//                     links: dep_list.iter().map(|d| CMakeTarget::from(d.clone())).collect(),
//                 }
//             }).collect::<Vec<CMakeList>>();
// 
//         let cmake_dependencies = cmake_lists.iter()
//             .flat_map(|l| l.links.iter())
//             .filter_map(|link| filter_match!(link, CMakeTarget::Library(lib), Some(lib.clone())))
//             .map(|lib| CMakeList {
//                 target: CMakeTarget::from(lib),
//                 links: Vec::new(),
//             }).collect::<Vec<CMakeList>>();
//         
//         // Sort lists by submdule 
//         let lists = submodules.iter()
//             .filter_map(|s| match cmake_lists.iter().find(|&l| l.target == s) {
//                 Some(lib) => Some((s.clone(), lib.clone())),
//                 None => None,
//             })
//             .collect::<HashMap<Submodule, CMakeList>>();
// 
//         dbg!(submodules.clone());
//         dbg!(cmake_lists.clone());
//         dbg!(cmake_dependencies.clone());
//         dbg!(lists);
// 
//         CMakeProject::new()
//     }
// }

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
