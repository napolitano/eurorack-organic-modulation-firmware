# Shared documentation assets

This directory contains repository-wide documentation assets that are not specific to the end-user manual.

## Drift heart

`drift-heart.svg` is the shared footer mark used by repository Markdown documentation. Its fill color is `#e64a2b`, matching the dominant orange-red accent used by the Drift manual graphics.

The footer itself is validated by `scripts/check_markdown_footer.py`. The GitHub pull-request template is intentionally excluded because template content is copied into every new pull request rather than rendered as project documentation.

## Installation assets

`installation/avrdudess-2.20-windows-numbered.png` is the numbered AVRDUDESS 2.20 / AVRDUDE 8.1 reference screenshot reused from the companion `napolitano/eurorack-quantizer-firmware` documentation. Drift reuses the same uploader controls, while its own installation guide supplies the Drift-specific bank filenames, bootloader choice, DIP mapping and power-safety instructions. The screenshot is a control-location reference only; the written settings are authoritative.

<!-- drift-footer:start -->
<p align="center">
  From Munich with <img src="drift-heart.svg" alt="an orange-red heart" width="16" height="16">
</p>
<!-- drift-footer:end -->
