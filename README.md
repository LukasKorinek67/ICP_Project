# ICP - semestrální práce
*Lukáš Kořínek, Helena Doanová*

---




### Instalace/Spuštění

- ?? exe soubor ?
- v rámci tohoto git repozitáře jsou zde 2 větve:

    - main - spustitelné na Windows
    - macOS_CLion - spustitelné na macOS v prostředí CLion

---
### Ovládání
- Pohyb/jízda:
  - <kbd>W</kbd> <kbd>S</kbd> <kbd>A</kbd> <kbd>D</kbd>
  - <kbd>⬆</kbd> <kbd>⬇</kbd> <kbd>⬅</kbd> <kbd>➡</kbd>
- Zrychlení (při pohybu vpřed) - <kbd>SPACE</kbd> 
- Nahoru - <kbd>P</kbd>
- Dolu - <kbd>L</kbd>
- Fullscreen ON/OFF - <kbd>F</kbd>
- Drive mode ON/OFF - <kbd>M</kbd>
- Vsync ON/OFF - <kbd>V</kbd>
- End - <kbd>Esc</kbd>
- Rozhlížení (mimo drive mode) - pohyb myši
- Přiblížení/oddálení - scroll myši

---
### Red colour tracker

- web kamera snimá červené objekty, při nalezení červeného objektu se změní barva semaforu na červenou, pokud není viditelný žádný červený objekt, zůstává defaultně zelená 

---
### Dokumentace

- Vytvořili jsme jednoduchou aplikaci, kde se dá přepínat mezi jízdou s formulí a volným pohybem
- Při jízdě je jiné chování kamery - zablokované pohyby myši
- Celkem 7 modelů - formule, kužely, kolo, drony, semafor, kostka/ohraničení mapy, samotná plocha a ??(transparentní) všude jsou textury
    - formule, drony a kolo se pohybují
    - ?? je transparentní
- Celkem 5 světel (1x DirLight, 3x PointLight, 1x SpotLight)
- Kolize jak pro formuli, tak pro kameru při volném pohybu
- Realtime 2D raster proccessing -> Red colour tracker, funkce popsaná výše


