# Islandora CWRC-Writer 

## Introduction

Provides a very minimal wrapper around the CWRC-Writer, so that it can be used in an islandora context.

## Requirements

This module requires the following modules/libraries:

* [Islandora](https://github.com/Islandora/islandora)
* [Islandora CWRC Document](https://github.com/cwrc/islandora_cwrc_document)
* [Islandora Rest](https://github.com/discoverygarden/islandora_rest)
* [Islandora Object Locking](https://github.com/discoverygarden/islandora_object_lock)
* [Libraries](https://www.drupal.org/project/libraries)
* [CWRC-Writer](https://github.com/cwrc/CWRC-Writer) [commit 811f7e0b8458f5899ce023551627a5376385a182 ](https://github.com/cwrc/CWRC-Writer/commit/811f7e0b8458f5899ce023551627a5376385a182)
* [jQuery Update](https://www.drupal.org/project/jquery_update) Version 1.8

jQuery Update is not a hard requirement but is necessary if you want to use the 
templates in the documents dialog box. octokit.js which is used to fetch the 
templates uses the global jQuery rather than using require.js to fetch the 
CWRC-Writer specific template.

CWRC-Writer is expected to be installed here:

* sites/all/libraries/CWRC-Writer (libraries directory may need to be created)

So far we've only tested up to CWRC-Writer [commit 811f7e0b8458f5899ce023551627a5376385a182 ](https://github.com/cwrc/CWRC-Writer/commit/811f7e0b8458f5899ce023551627a5376385a182) with islandora_cwrc_writer [commit 810970bcb588a53e907ba306dd362e3bffd89c2a](https://github.com/cwrc/islandora_cwrc_writer/commit/810970bcb588a53e907ba306dd362e3bffd89c2a)

### Java Servlet Configuration

CWRC-Writer depends on a number of Java Servlets to be functional.

* [cwrc-validator](https://github.com/cwrc/cwrc-validator)

Follow the instructions on the page, but also *rename* the generated war to
validator.war. In most cases your tomcat should exist here
_/usr/local/fedora/tomcat_.

### Reverse proxy config:

We make the assumption that we (reverse) proxy VIAF, to fix the same-origin
issue.

For Apache, with Drupal running on the same box as Apache, a couple lines like:

```
ProxyPass /viaf http://www.viaf.org/viaf
ProxyPassReverse /viaf http://www.viaf.org/viaf
```

To be able to validate documents, we require that the validator.war is deployed
to your tomcat directory, and that you set up a (reverse) proxy so that the
CWRC-Writer can communicate with it.

```
ProxyPass /cwrc/services/validator/ http://localhost:8080/validator/
ProxyPassReverse /cwrc/services/validator/ http://localhost:8080/validator/
```

To be able to access Geonames service you must set up a proxy with 
authentication: 

```
<Location /geonames>
   RequestHeader set Authorization "Basic XXXXX"
   ProxyPass http://apps.testing.cwrc.ca/cwrc-mtp/geonames/
   ProxyPassReverse http://apps.testing.cwrc.ca/cwrc-mtp/geonames/
</Location>
```

You'll need permission / authentication credentials from the 
CWRC organization. You can generation the credentials (replaces the XXXXX 
portion above) like so:

```
echo -n "username:password" | base64
```

In addition you must also enable mod_headers for the authentication 
credentials to be passed on to the CWRC Geonames service. With apache2 on 
Ubuntu this can be done like so:

```
sudo a2enmod headers
sudo service apache2 restart
```

## Touchpoints between CWRC-Writer (JavaScript) and Islandora (cwrc/islandora_cwrc_writer Drupal Module)

Section goals: 
* communicate the assumptions built into the integration module about how to interact with CWRC-Writer
* help determine whether or not a change to CWRC-Writer will be a breaking change to the integration module
* be updated as CWRC-Writer changes

The islandora_cwrc_writer modules uses code within the "theme", "utilities", and "js" directories to embed CWRC-Writer into a Drupal page by referencing items within the CWRC-Writer directory and overriding aspects (delegator to save/load docs).


Touchpoints rev. 2017-08-22 - CWRC-Writer [commit 811f7e0b8458f5899ce023551627a5376385a182 ](https://github.com/cwrc/CWRC-Writer/commit/811f7e0b8458f5899ce023551627a5376385a182) with islandora_cwrc_writer [commit 810970bcb588a53e907ba306dd362e3bffd89c2a](https://github.com/cwrc/islandora_cwrc_writer/commit/810970bcb588a53e907ba306dd362e3bffd89c2a). *Note: These are untested with the NPM version of CWRC-Writer and will need to be updated*

1. islandora_cwrc_writer module loads files from within the CWRC-Writer library
    * CWRC-Writer treated as a self-contained library with the Drupal module loading specific files from specified internal CWRC-Writer directory locations (e.g., JS, CSS, etc)
    * If directories or filenames are changed then the Islandora integrated CWRC-Writer will break. Examples include
      * https://github.com/cwrc/islandora_cwrc_writer/blob/7.x/js/README.md
      * https://github.com/cwrc/islandora_cwrc_writer/blob/7.x/islandora_cwrc_writer.module#L12
      * https://github.com/cwrc/islandora_cwrc_writer/blob/7.x/js/islandora_cwrc_writer.js#L26-L29
      * https://github.com/cwrc/islandora_cwrc_writer/blob/7.x/includes/utilities.inc#L164-L165
      * https://github.com/cwrc/islandora_cwrc_writer/blob/7.x/includes/utilities.inc#L1091-L1097
      * https://github.com/ajmacdonald/islandora_cwrc_writer/blob/7.x/islandora_cwrc_writer.install#L17
      * https://github.com/cwrc/islandora_cwrc_writer/blob/7.x/islandora_cwrc_writer.module#L262
      * https://github.com/cwrc/islandora_cwrc_writer/blob/7.x/islandora_cwrc_writer.module#L275-L286
      * list may not be exhustive - untested 
    * ToDo: consider creating a cleaner means such that the number of locations to change is reduced
    
2. Config parameters passed to CWRC-Writer
    * including: https://github.com/cwrc/CWRC-Writer/blob/811f7e0b8458f5899ce023551627a5376385a182/src/js/writerConfig.js
      * set by
        * https://github.com/cwrc/CWRC-Writer/blob/811f7e0b8458f5899ce023551627a5376385a182/src/js/writerConfig.js
        * https://github.com/cwrc/islandora_cwrc_writer/blob/7.x/includes/utilities.inc#L1051-L1127
      * passed by
        * https://github.com/cwrc/islandora_cwrc_writer/blob/7.x/js/islandora_cwrc_writer.js

3. Instantiate CWRC-Writer within the Drupal page
    * https://github.com/cwrc/islandora_cwrc_writer/blob/7.x/js/islandora_cwrc_writer.js
    * https://github.com/cwrc/islandora_cwrc_writer/blob/58ed10b4e25906cff75e12778a0706026d25d815/js/islandora_cwrc_writer.js#L325-L378
  
4. Create delegator for load/save operations
    * https://github.com/cwrc/islandora_cwrc_writer/blob/58ed10b4e25906cff75e12778a0706026d25d815/js/islandora_cwrc_writer.js#L17-L18
    * https://github.com/cwrc/islandora_cwrc_writer/blob/58ed10b4e25906cff75e12778a0706026d25d815/js/islandora_cwrc_writer.js#L126-L160
    * https://github.com/cwrc/islandora_cwrc_writer/blob/58ed10b4e25906cff75e12778a0706026d25d815/js/islandora_cwrc_writer.js#L163-L197
    * https://github.com/cwrc/islandora_cwrc_writer/blob/58ed10b4e25906cff75e12778a0706026d25d815/js/islandora_cwrc_writer.js#L163-L197
  
5. Override CWRC-Writer Code to add new features
    * schemaId needs to be passed into specific functions as CWRC-Writer assume each schema can have only one CSS while the Islandora overrides this limitation
      * https://github.com/cwrc/islandora_cwrc_writer/blob/58ed10b4e25906cff75e12778a0706026d25d815/js/islandora_cwrc_writer.js#L325-L378
      * GitHub issue: https://github.com/cwrc/CWRC-Writer/issues/215
      * Also in the Notes embedded CWRC-Writer
        * https://github.com/cwrc/islandora_cwrc_writer/blob/58ed10b4e25906cff75e12778a0706026d25d815/js/islandora_cwrc_writer_note.js
  
## To Do

* Look into integrating the [Geonames Service](http://github.com/cwrc/CWRC-Mapping-Timelines-Project/tree/master/geonames)

## Troubleshooting/Issues

Having problems or solved a problem? Contact [discoverygarden](http://support.discoverygarden.ca).

## Maintainers/Sponsors

Current maintainers:

* [discoverygarden](http://wwww.discoverygarden.ca)

## Development

If you would like to contribute to this module, please check out our helpful
[Documentation for Developers](https://github.com/Islandora/islandora/wiki#wiki-documentation-for-developers)
info, [Developers](http://islandora.ca/developers) section on Islandora.ca and
contact [discoverygarden](http://support.discoverygarden.ca).

## License

[GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)

