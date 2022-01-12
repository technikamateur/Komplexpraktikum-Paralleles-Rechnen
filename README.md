# Komplexpraktikum-Paralleles-Rechnen-WiSe-2021-22
## Aufgabe B
Implementieren Sie eine thread-parallele Variante von Conways "Game-of-Life" mit periodic boundary conditions. Nutzen Sie dazu OpenMP Compiler-Direktiven. Benutzen Sie double buffering um Abhängigkeiten aufzulösen.

- Beschreiben Sie Ihren Ansatz und gehen Sie sicher, dass die Arbeit thread-parallel ausgeführt wird.
- Messen und Vergleichen Sie die Ausführungszeiten für 1,2,4,8,16 und 32 Threads, für den GCC, als auch den Intel Compiler bei Feldgrößen von 128x128, 512x512, 2048x2048, 8192x8192 und 32768x32768.
- Nutzen Sie für die Berechnung eine geeignete Anzahl an Schleifendurchläufen (Zyklen des Spiels).
- Nutzen Sie dafür die "romeo" Partition von taurus.
- Achten Sie darauf, dass benachbarte Threads möglichst nah einander gescheduled sind.
- Testen Sie für die Feldgröße 128x128, welchen Einfluss die OpenMP Schleifenschedulingverfahren haben (OMP_SCHEDULE), indem Sie für die Ausführung mit 32-Threads des mit Intel kompilierten Benchmarks die Verfahren static, dynamic, guided, und auto bei default chunk size vergleichen
- Führen Sie jeweils 20 Messungen durch und analysieren Sie die Ergebnisse mit geeigneten statistischen Mitteln.

## Aufgabe C
Implementieren Sie eine SIMD-parallele Variante von Conways "Game-of-Life" mit periodic boundary conditions. Nutzen Sie dazu OpenMP Compiler-Direktiven. Benutzen Sie double buffering um Abhängigkeiten aufzulösen.

- Beschreiben Sie Ihren Ansatz und gehen Sie sicher, dass die Arbeit SIMD-parallel ausgeführt wird.
- Messen und Vergleichen Sie die Ausführungszeiten für aktiviertes und inaktives OpenMP, sowie den GCC, als auch den Intel Compiler bei Feldgrößen von 128x128, 512x512, 2048x2048, 8192x8192 und 32768x32768.
- Nutzen Sie für die Berechnung eine geeignete Anzahl an Schleifendurchläufen (Zyklen des Spiels).
- Nutzen Sie dafür die "romeo" Partition von taurus.
- Führen Sie jeweils 20 Messungen durch und analysieren Sie die Ergebnisse mit geeigneten statistischen Mitteln.

## Aufgabe D
Implementieren Sie eine MPI-parallele Variante von Conways "Game-of-Life" mit periodic boundary conditions. Nutzen Sie für die Synchronisierung zwischen den einzelnen Ranks asynchrone Kommunikation (z.B. MPI_Isend). Benutzen Sie double buffering um Abhängigkeiten aufzulösen.
- Beschreiben Sie Ihren Ansatz und gehen Sie sicher, dass die der Datenaustausch effizient ausgeführt wird.
- Teilen Sie die Sie die Felder nach den Schemas blockweise (Blöcke von Zeilen) und chessboard (block/block) auf die ranks auf.
- Messen und vergleichen Sie die Ausführungszeiten für 1,4,16,64,128 und 256 ranks bei Feldgrößen von 2048x2048, 8192x8192, 32768x32768, 131072x131072 (nur ab 64 ranks ), wobei die gesamten Felder auf die ranks aufgeteilt werden.
- Nutzen Sie für die Berechnung eine geeignete Anzahl an Schleifendurchläufen (Zyklen des Spiels).
- Achten Sie darauf, dass benachbarte Ranks möglichst nah beieinander gescheduled sind.
- Nutzen Sie dafür die "romeo" Partition von taurus.
- Führen Sie jeweils 20 Messungen durch und analysieren Sie die Ergebnisse mit geeigneten statistischen Mitteln.
