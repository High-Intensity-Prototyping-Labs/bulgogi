// CMake module

use crate::project::Project;

pub struct CMakeProject {
    pub targets: Vec<CMakeTarget>
}

pub struct CMakeTarget {
    pub kind: CMakeTargetKind,
}

pub enum CMakeTargetKind {
    LibModule,
    LibTarget,
    ExeTarget,
}

impl CMakeProject {
    pub fn new() -> Self {
        CMakeProject {
            targets: vec![],
        }
    }
}

impl From<Project> for CMakeProject {
    fn from(project: Project) -> Self {
        let mut cmake_project = CMakeProject::new();

        for target in project.targets {
            
        }

        cmake_project
    }
}
