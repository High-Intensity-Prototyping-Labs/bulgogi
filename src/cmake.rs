// CMake module

use crate::project::Project;
use crate::target::Target;
use crate::dependency::DepKind;

/// The largest unit the CMake module understands.
/// Designed to be interchangeable with bulgogi projects.
pub struct CMakeProject {
    pub lists: Vec<CMakeList>
}

/// Grouping of CMakeTargets -- these are better representations of actual compiled targets.
/// (i.e.: every executable (.out) and library (.a) will have a CMakeList of CMakeTargets that are
/// linked together.
pub struct CMakeList {
    pub targets: Vec<CMakeTarget>,
}

/// Distinguishes libraries and executables apart.
/// The first string is always the module (dir) name, the second is always the proxy target name.
/// These two will often be the same value in the case of LibModules.
pub enum CMakeTarget {
    Library(String, String),
    Executable(String, String),
}
