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
  <div id="cwrc_header" class="cwrc ui-layout-north">
    <h1><?php print $title; ?></h1>
    <?php print $header; ?>
  </div>
  <div class="cwrc ui-layout-west">
    <div id="westTabs" class="tabs">
      <?php print $western_tabs; ?>
      <div id="westTabsContent" class="ui-layout-content">
        <?php print $western_tabs_content; ?>
      </div>
    </div>
  </div>
  <div id="cwrc_main" class="ui-layout-center">
    <div class="ui-layout-center">
      <form method="post" action="">
        <textarea id="editor" name="editor" class="tinymce"></textarea>
      </form>
    </div>
    <div class="cwrc ui-layout-south">
      <div id="southTabs" class="tabs">
        <?php print $southern_tabs; ?>
        <div id="southTabsContent" class="ui-layout-content"></div>
      </div>
    </div>
  </div>
  <!-- The eastern panel is optional and may not be provided -->
  <?php if (isset($eastern_panel)): ?>
    <div class="ui-layout-east">
      <?php print $eastern_panel; ?>
    </div>
  <?php endif; ?>
</div>
