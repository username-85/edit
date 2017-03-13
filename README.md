 edit - text editor in C using ncurses library.  
Was written only in educational purposes.  


 What i've tried to implement:


 Some utf8 support (input/output).  
Gap buffer data structure.  
Display buffer in separate function and not keeping terminal coordinates of 
cursor in buffer data structure.


Hotkeys:


F1           - Help  
F2           - Save  
F3           - Selection toggle  
F4           - Copy selected text  
F5           - Cut selected text  
F6           - Paste selected text  
F10          - Quit  
Esc          - Cancel selection mode  
Home         - Move cursor to start of current line  
End          - Move cursor to end of current line  
PageUp       - move cursor few lines up  
PageDown     - move cursor few lines down  
Delete       - delete symbol under cursor  
Backspace    - delete symbol before cursor  
pointer keys - cursor movement  


I've only tested it on Linux in urxvt terminal with en\_US.UTF-8 locale. 
Multybyte input/output was tested on cyrillic characters.


Known bugs:


In file saving input some non ascii characters could look weird after cursor 
movent . To fix it i probably should rewrite that input without using ncurses 
getstr function.  
PageUp/PageDown may scroll too far in files with some long lines.
