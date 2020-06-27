# Projekt programowanie niskopoziomowe

## Plan działania

- Wytrenowanie modelu tensorflow rozpozającego włamywacza. 
- Stworzenie aplikacji w c++, która będzie:
  - zbierać zdjęcia z kamery.
  - sprawdzać czy jest na nich włamywacz za pomocą modelu tensorflow.
  - Wysyłać pozytywnie przewidziane zdjęcia. (Klient tcp)
- Stworzenie komunikacji tcp między aplikacją c++ oraz aplikacją na raspberryPi w tej samej lokalnej sieci. (Przesyłwanie zdjęć)
- Stworzenie aplikacji w pythonie na raspberryPi która będzie: 
  - Odbierać zdjęcia.(Serwer tcp)
  - Zapisywać zdjęcia na dysku rasppberryPi.
- Stworzenie serwera www na raspberryPi oraz strony internetowej na której będą wyświetlać się zdjęcia 

app that uses tensorflow to detect objects on camera and than sending detected data through tcp to server. 

Tworzenie serwera www na raspberry. 
https://www.raspberrypi.org/documentation/remote-access/web-server/apache.md

## 1. Wytrenowanie modelu tensorflow rozpozającego włamywacza. 

## 2. sdfsd
