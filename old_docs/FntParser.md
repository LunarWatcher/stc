# FntParser

Elementary parser for .fnt files aimed at use with OpenGL. May in the future expand to an all-round font management library.

## FntParser

The primary and currently only parser for this module in general is the FntParser; the parser for .fnt files.

Note, however, that all it does is load and parse the .fnt files. No other information whatsoever is loaded; this includes the pages the various associated filetypes that can be used to store the fonts (such as png). Loading these (correctly) is your job.

### Data types

See the source code for the documentation for the structs themselves.

#### FntInfo

Contains information about the font itself, including info about the stored characters. Contains generic information about the pages, as well as the font attributes.

#### FntCharInfo

Contains info about a specific character, particularly its position and size in the associated atlas, as well as the page number.

### Functions

#### `FntInfo loadAndParseFnt(std::string fileName)`

Loads the file, and parses it. Returns FntInfo with all the data loaded.

Note that not all the data is included at this time, because it was intended for a specific program, so I cut some corners. If there's fields you're missing that are either not defined or not loaded, consider submitting a PR, or opening an issue.
