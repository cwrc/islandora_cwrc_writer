window.Drupal = window.Drupal || {};

let cwrc = require('cwrc-tei-entities-lookup');
cwrc.setEntityRoot('https://commons.cwrc.ca');
cwrc.setSearchRoot(Drupal.settings.CWRCWriter.cwrcDialogs.cwrcApiUrl);
let viaf = require('viaf-entity-lookup');
let dbpedia = require('dbpedia-entity-lookup');
let wikidata = require('wikidata-entity-lookup');
let getty = require('getty-entity-lookup');
let geonames = require('geonames-entity-lookup');

window.Drupal.CWRCWriterDialogs = require('cwrc-public-entity-dialogs');
window.Drupal.CWRCWriterDialogs.showNoLinkButton(true);
window.Drupal.CWRCWriterDialogs.showCreateNewButton(true);
window.Drupal.CWRCWriterDialogs.showEditButton(true);
window.Drupal.CWRCWriterDialogs.setEntityFormsRoot(Drupal.settings.CWRCWriter.cwrcDialogs.entityFormsRootUrl);
window.Drupal.CWRCWriterDialogs.setCollectionsRoot(Drupal.settings.CWRCWriter.cwrcDialogs.entityFormsCollectionUrl);
window.Drupal.CWRCWriterDialogs.registerEntitySources({
    person: (new Map()).set('cwrc', cwrc).set('viaf', viaf).set('wikidata', wikidata).set('getty', getty).set('dbpedia', dbpedia),
    place: (new Map()).set('cwrc', cwrc).set('geonames', geonames).set('viaf', viaf).set('dbpedia', dbpedia).set('wikidata', wikidata),
    organization: (new Map()).set('cwrc', cwrc).set('viaf', viaf).set('wikidata', wikidata).set('dbpedia', dbpedia),
    title: (new Map()).set('cwrc', cwrc).set('viaf', viaf).set('wikidata', wikidata).set('dbpedia', dbpedia)
})

window.Drupal.CWRCWriter = require('cwrc-writer-base');

