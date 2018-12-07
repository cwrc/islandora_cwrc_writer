# Islandora CWRC-Writer 

## Introduction

Provides a very minimal wrapper around the [CWRC-Writer](https://github.com/cwrc/CWRC-WriterBase), so that it can be used in an Islandora context.

## Requirements

This module requires the following modules/libraries:

* [Islandora](https://github.com/Islandora/islandora)
* [Islandora CWRC Document](https://github.com/cwrc/islandora_cwrc_document)
* [Islandora Rest](https://github.com/discoverygarden/islandora_rest)
* [Islandora Object Locking](https://github.com/discoverygarden/islandora_object_lock)
* [Libraries](https://www.drupal.org/project/libraries)

The [build](https://github.com/cwrc/Islandora-CWRC-Writer/tree/master/build) directory contains the compiled version of the Islandora CWRC-Writer. It can also be [built from the source](https://github.com/cwrc/Islandora-CWRC-Writer/blob/master/package.json#L21), if [NPM](https://github.com/npm/cli) is installed.

Islandora CWRC-Writer is expected to be installed here:

* sites/default/modules/islandora_cwrc_writer/

## Installation through NPM

1. Navigate to `sites/default/modules/islandora_cwrc_writer/`
2. Clone this repo: `git clone https://github.com/cwrc/islandora_cwrc_writer.git`
3. Install the dependencies: `npm install`
4. Run the build: `npm run build`

## Updating

1. Navigate to `sites/default/modules/islandora_cwrc_writer/`
2. Get the latest changes: `git pull`

## Touchpoints between CWRC-Writer (JavaScript) and Islandora (cwrc/islandora_cwrc_writer Drupal Module)

Section goals: 
* communicate the assumptions built into the integration module about how to interact with CWRC-Writer
* help determine whether or not a change to CWRC-Writer will be a breaking change to the integration module
* be updated as CWRC-Writer changes

The islandora_cwrc_writer modules uses code within the "theme", "utilities", and "js" directories to embed CWRC-Writer into a Drupal page by referencing items within the CWRC-Writer directory and overriding aspects (delegator to save/load docs).

1. islandora_cwrc_writer module loads files from within the CWRC-Writer library
    * CWRC-Writer treated as a self-contained library with the Drupal module loading specific files from specified internal CWRC-Writer directory locations (e.g., JS, CSS, etc)
    * If directories or filenames are changed then the Islandora integrated CWRC-Writer will break. Examples include
      * https://github.com/cwrc/islandora_cwrc_writer/blob/master/js/README.md
      * https://github.com/cwrc/islandora_cwrc_writer/blob/master/islandora_cwrc_writer.module#L12
      * https://github.com/cwrc/islandora_cwrc_writer/blob/master/includes/utilities.inc#L164-L165
      * https://github.com/cwrc/islandora_cwrc_writer/blob/master/includes/utilities.inc#L1091-L1097
      * https://github.com/cwrc/islandora_cwrc_writer/blob/master/islandora_cwrc_writer.install#L17
      * https://github.com/cwrc/islandora_cwrc_writer/blob/master/islandora_cwrc_writer.module#L274
      * list may not be exhustive - untested
    
2. Config parameters passed to CWRC-Writer
    * CWRC-GitWriter example: https://github.com/cwrc/CWRC-GitWriter/blob/master/src/js/config.js
      * set in islandora_cwrc_writer module by
        * https://github.com/cwrc/Islandora-CWRC-Writer/blob/33c1209987709779fe41671ea2f3ae2d6d24697a/includes/utilities.inc#L1088-L1131
      * and passed to CWRC-Writer by
        * https://github.com/cwrc/Islandora-CWRC-Writer/blob/33c1209987709779fe41671ea2f3ae2d6d24697a/js/islandora_cwrc_writer.js#L101

3. Instantiate CWRC-Writer within the Drupal page
    * https://github.com/cwrc/Islandora-CWRC-Writer/blob/33c1209987709779fe41671ea2f3ae2d6d24697a/js/islandora_cwrc_writer.js#L233-L271
    
4. Load and save the document
    * https://github.com/cwrc/Islandora-CWRC-Writer/blob/33c1209987709779fe41671ea2f3ae2d6d24697a/js/islandora_cwrc_writer.js#L114-L171

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

