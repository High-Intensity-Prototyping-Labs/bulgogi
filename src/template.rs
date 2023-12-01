// Template module

use handlebars::Handlebars;
use std::fs;
use serde_json::json;

pub fn load() -> Result<(), Box<dyn std::error::Error>> {
    let mut reg = Handlebars::new();
    let template = fs::read_to_string("templates/top.blgg").expect("required");
    let values_f = fs::File::open("templates/default.json").expect("required");
    let values: serde_json::Value = serde_json::from_reader(values_f).expect("required");
    
    println!(
        "{}",
        reg.render_template(&template, &values)?
    );

    Ok(())
}
