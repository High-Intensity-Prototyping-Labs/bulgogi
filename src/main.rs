// Root module
mod project;
//mod target;
//mod dependency;
//mod client;
//mod template;
//mod cmake;

use std::io;
use std::fs;

use serde_yaml::Mapping;
use project::Project;

fn main() -> Result<(), io::Error> {
//    client::next_command()?;
    let file = fs::File::open("project.yaml")?;
    let map: Mapping = serde_yaml::from_reader(file).unwrap();
    let project = Project::from(map);
    Ok(())
}
