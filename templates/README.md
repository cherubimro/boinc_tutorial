# BOINC XML Templates

This directory contains the XML template files needed by BOINC server.

## Files

### pi_in.xml - Input Template
Defines the structure of input files for work units. This tells BOINC:
- Which files to send to clients
- What to name them locally on the client
- File numbering scheme

The `<file_number>0</file_number>` refers to the file specified when creating work with `create_work`.

### pi_out.xml - Output Template
Defines the structure of result files returned by clients. This specifies:
- Output file naming pattern (`<OUTFILE_0/>` gets replaced with actual filename)
- When to upload (when file is present)
- Maximum file size
- Upload URL (automatically filled by BOINC)

### version.xml - Application Version
Defines the application binary and its properties:
- Physical filename on server
- Whether it's the main program or a library
- Optional: signature for code signing
- Optional: file_prefix for multiple files

This file goes in: `projects/apps/pi_compute/1.0/x86_64-pc-linux-gnu/version.xml`

## Usage

These templates are used by BOINC tools:

```bash
# Create work unit using templates
bin/create_work --appname pi_compute \
    --wu_name pi_wu_1 \
    --wu_template templates/pi_in.xml \
    --result_template templates/pi_out.xml \
    --target_nresults 3 \
    input_file.txt
```

The setup script automatically copies these to the correct locations.

## Customization

You can modify these templates for different application needs:

- **Multiple input files**: Add more `<file_info>` and `<file_ref>` blocks
- **Different file sizes**: Change `<max_nbytes>`
- **Sticky files**: Add `<sticky/>` to keep files on client
- **Optional files**: Add `<optional/>` for files that may not exist

See BOINC documentation for full template syntax:
https://github.com/BOINC/boinc/wiki/XmlTemplates
