window.Drupal = window.Drupal || {};

let cwrc = require('cwrc-entity-lookup');
cwrc.setSearchRoot(Drupal.settings.CWRCWriter.cwrcDialogs.cwrcApiUrl);
let viaf = require('viaf-entity-lookup');
let dbpedia = require('dbpedia-entity-lookup');
let wikidata = require('wikidata-entity-lookup');
let getty = require('getty-entity-lookup');
let geonames = require('geonames-entity-lookup');

window.Drupal.CWRCWriterDialogs = require('cwrc-public-entity-dialogs');
window.Drupal.CWRCWriterDialogs.registerEntitySources({
    people: (new Map()).set('cwrc', cwrc).set('viaf', viaf).set('wikidata', wikidata).set('getty', getty).set('dbpedia', dbpedia),
    places: (new Map()).set('cwrc', cwrc).set('geonames', geonames).set('viaf', viaf).set('dbpedia', dbpedia).set('wikidata', wikidata),
    organizations: (new Map()).set('cwrc', cwrc).set('viaf', viaf).set('wikidata', wikidata).set('dbpedia', dbpedia),
    titles: (new Map()).set('cwrc', cwrc).set('viaf', viaf).set('wikidata', wikidata).set('dbpedia', dbpedia)
})

window.Drupal.CWRCWriter = require('cwrc-writer-base');

