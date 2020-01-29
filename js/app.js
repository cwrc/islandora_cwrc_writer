window.Drupal = window.Drupal || {};

const cwrc = require('cwrc-tei-entities-lookup');
cwrc.setEntityRoot(Drupal.settings.CWRCWriter.cwrcDialogs.repositoryBaseObjectUrl);
cwrc.setSearchRoot(Drupal.settings.CWRCWriter.cwrcDialogs.cwrcApiUrl);
cwrc.setProjectLookupConfig({
    projectLookupUrl: Drupal.settings.CWRCWriter.cwrcDialogs.projectLookupUrl,
    projectLogoRoot: Drupal.settings.CWRCWriter.cwrcDialogs.projectLogoRoot,
    cwrcProjectId: Drupal.settings.CWRCWriter.cwrcDialogs.cwrcProjectId
});

const viaf = require('viaf-entity-lookup')
const dbpedia = require('dbpedia-entity-lookup');
const wikidata = require('wikidata-entity-lookup');
const getty = require('getty-entity-lookup');
const geonames = require('geonames-entity-lookup');

const EntityLookupDialogs = require('cwrc-public-entity-dialogs');
EntityLookupDialogs.showNoLinkButton(true);
EntityLookupDialogs.showCreateNewButton(true);
EntityLookupDialogs.setEntityCollectionsUrl(window.location.origin+'/islandora/get_entity_collections');
EntityLookupDialogs.showEditButton(true);
EntityLookupDialogs.setEntityFormsRoot(Drupal.settings.CWRCWriter.cwrcDialogs.entityFormsRootUrl);
EntityLookupDialogs.setCollectionsRoot(Drupal.settings.CWRCWriter.cwrcDialogs.entityFormsCollectionUrl);
EntityLookupDialogs.registerEntitySources({
    person: (new Map()).set('cwrc', cwrc).set('viaf', viaf).set('wikidata', wikidata).set('getty', getty).set('dbpedia', dbpedia),
    place: (new Map()).set('cwrc', cwrc).set('geonames', geonames).set('viaf', viaf).set('dbpedia', dbpedia).set('wikidata', wikidata),
    organization: (new Map()).set('cwrc', cwrc).set('viaf', viaf).set('wikidata', wikidata).set('dbpedia', dbpedia),
    title: (new Map()).set('cwrc', cwrc).set('viaf', viaf).set('wikidata', wikidata).set('dbpedia', dbpedia)
})

window.Drupal.CWRCWriterDialogs = EntityLookupDialogs;

window.Drupal.CWRCWriter = require('cwrc-writer-base');
