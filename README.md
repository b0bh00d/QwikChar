<p align="center">
  <a href="https://rclone.org/">
    <img width="20%" alt="Reactor" src="https://user-images.githubusercontent.com/4536448/174499720-c3b572e1-d9c7-405c-b621-e08c64bb919a.png">
  </a>
</p>

# QwikChar
A quick-select pop-up panel for Unicode characters

## Summary
This project was a personal coding challenge, prompted by a [reddit post](https://www.reddit.com/r/windows/comments/p6h6bh/i_made_a_concept_for_a_character_selector_by/) I stumbled upon one day while visiting various rabbit holes.

## Ya Know, Code
Although this started as just a coding challenge, there's definitely something to be said for shortening the route to selecting Latin and Greek characters (or any other non-standard glyph).  On Windows, you have to run through a lot of menus to open the Character Map, and then jump through its hoops just to get a Unicode character onto the system clipboard.  `QwikChar` makes the process almost instant, and adds a little value on top.

### Activation
The default activation sequence is `Windows key + Z`.  Once you open the panel for the first time, you can open the settings and assign a different sequence.  The current code will work with Latin A-Z, and function keys (F1-F12).  You can use combinations of Ctrl, Alt and Shift with most, or you can use the Windows key, the latter being an either-or:  You cannot combine the Windows key with the other modifiers.

Selecting a character will automatically close the panel (your selected character will be placed on the system clipboard).  If, however, you decide not to select, you can close the panel at any time by pressing the `Escape` key.

### Default Character Set
The default character set provides glyphs for Latin and Greek vowels.

![QwikChar_panel](https://user-images.githubusercontent.com/4536448/174499723-0bd1105b-0ac9-44fe-99ed-05f87ab15b10.png)

Once the panel is open, you can press a vowel character on your keyboard, and the selection will instantly scroll to the first writing system for that section.  Repeatedly pressing the same character will cycle through all writing systems for that character.

The character selection that comes with `QwikChar` is just the default, however.  By copying the `glyph.xml` file into your user settings folder (%LOCALAPPDATA%\QwikChar on Windows), you can actually customize the characters that are presented on the panel.  You can use any Latin alphabet character ('a' to 'z') to identify a section, and then any number of writing systems and glyphs therein.  As an example, you could insert the following new 'z' section into your customized `glyph.xml` file:

```
  <key value="z">
    <system name="Emoticons">
      <!-- glyphs can be multi-character, up to ten characters in length -->
      <glyph value="¯\_(ツ)_/¯"/> <!-- Donger -->
      <glyph value="😂"/>        <!-- Unicode emote -->
      <glyph value="ʕ•ᴥ•ʔ╭∩╮"/>   <!-- Donger -->
    </system>
  </key>
```

and you would then have a new section where you can select more elaborate (and fun) glyphs.

### MRU
As you select characters from the panel, they are placed on the system clipboard for you to paste into any document you wish.  The `QwikChar` program remembers the last glyph you selected, and places it into a most-recently-used (MRU) list at the bottom of the panel, aging from left to right.

![QwikChar_mru](https://user-images.githubusercontent.com/4536448/174499722-cabdb7d9-0711-4707-b1d9-d2590569e7b6.png)

This way, oft-used glyphs are instantly accessible without searching.

## Notes
* `QwikChar` is farily minimal in its feature set, so if you want it launched at system start-up, you'll have to do so manually (add a shortcut file to the "Startup" folder, create a Task Scheduler entry, etc.).
* The program is primiarly intended for use on Microsoft Windows, but could be ported to another platform with a little effort.
* If there's any interest, I can update the project to perform 'auto-type' of the selected glyph into the last-active window.
* `QwikChar` was developed using Qt 5.14.1.

## You're Welcome, reddit
I hope you find `QwikChar` useful.