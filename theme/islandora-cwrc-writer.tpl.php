<?php
/**
 * @file
 * Renders the CWRC-Writer.
 *
 * Variables available:
 * - $title: Title of to display in the CWRC-Writer Header.
 * - $header: Any additional HTML to render in the header.
 * - $western_tabs: Links to use as the tabs headers in the western pane, three
 *   are expected by the CWRC-Writer (Entities / Structure / Relations), their
 *   content gets populated by the CWRC-Writer.
 * - $western_tabs_content: The div's that make up each western tabs content.
 * - $southern_tabs: Links to use as the tabs headers in the southern pane two
 *   are expected by the CWRC-Writer (Selection / Validation), their
 *   content gets populated by the CWRC-Writer. No others are supported at this
 *   time.
 * - $eastern_panel: (Optional) HTML content to embed in the eastern panel,
 *   typically the Image Annotation Viewer or JWPlayer.
 *
 * @see template_preprocess_islandora_cwrc_writer()
 * @see template_process_islandora_cwrc_writer()
 */
?>
<div id="cwrc_wrapper">
</div>
