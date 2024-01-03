// Dependency Module

pub const MODULE_DEFAULT: &str = "module1";

/// Target dependency type
#[derive(Debug, Clone, PartialEq)]
pub struct Dependency {
    pub name: String,
    pub kind: DepKind,
    pub flag: DepFlag,
}

#[derive(Debug, Clone, PartialEq)]
pub enum DepKind {
    Target,
    Module,
}

#[derive(Debug, Clone, PartialEq)]
pub enum DepFlag {
    None,
    Executable,
}

impl From<(String, DepKind)> for Dependency {
    fn from(value: (String, DepKind)) -> Self {
        Dependency {
            name: value.0,
            kind: value.1,
            flag: DepFlag::None,
        }
    }
}

impl From<(String, DepKind, DepFlag)> for Dependency {
    fn from(value: (String, DepKind, DepFlag)) -> Self {
        Dependency {
            name: value.0,
            kind: value.1,
            flag: value.2,
        }
    }
}
