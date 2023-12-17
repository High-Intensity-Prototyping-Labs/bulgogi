// Root module
mod project;
//mod target;
//mod dependency;
mod client;
mod template;
//mod cmake;

use std::io;
fn main() -> Result<(), io::Error> {
    client::next_command()?;
    Ok(())
}
