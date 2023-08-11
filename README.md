bulgogi -- a flexible build system assistant.
v1.0.0

# Brief

Fundamentally, most programming projects will (and can) adhere to fundamentally similar file organizations.

Differences tend to reflect developer preferences and specific technical requirements imposed by the build system or target systems.

Bulgogi intends to standardize every step of the development process from a build system and deployment standpoint. 

From initializing the project, to adding modules and mixing languages to targeting specific platforms, bulgogi makes this otherwise infinitely painful process painless.

# Templates

Currently, bulgogi supports deploying templates to kick-start development projects.

Templates exist as filesystem layouts. Empty folders are treated as necessary 'bins' or placeholders for files to-be generated. All other files are seen as necessary working components of the base project.

Consider the following sample template:

```
template/
|- src/
|- inc/
|- obj/
|- makefile
```

Here, 3 empty folders designate the layout for the corresponding source, header and object files respectively.

Lastly, the makefile is pre-programmed to first compile every .c file it finds in src/ to a corresponding .o file into obj/, and searches for header files in inc/.

Bulgogi can read this template directory and understand how to deploy it in a project environment.

# Initializing a Project from a Template 

Running the following command will deploy the default template known to bulgogi:

```
bulgogi init 
```

This will initialize the current working directory by default. This is equivalent to:

```
bulgogi init .
```

Whereby . denotes the current directory, but could be any directory of choice.
