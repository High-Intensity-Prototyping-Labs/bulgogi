// Dependency Module

pub const MODULE_DEFAULT: &str = "module1";

pub type Dependency = (String, DepKind);

#[derive(Debug, Clone, PartialEq)]
pub enum DepKind {
    Target,
    Module,
    ModuleExe,
}
