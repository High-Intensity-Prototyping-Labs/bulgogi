// Dependency Module

pub const MODULE_DEFAULT: &str = "module1";

pub type Dependency = (String, DepKind);

#[derive(Debug)]
pub enum DepKind {
    Target,
    Module,
}
