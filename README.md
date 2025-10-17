# Segfault-Game-Engine
A game engine using NVRHI and SDL3. Intended as a learning project. Do NOT use this in any real capacity. The name alone should be a clue!

## Configurations Speed using Crappy AI generated Benchmark
### cheap_hash


|20 Runs, 10 Mil batch | Debug          | Release       | Distrubtion   |
|----------------------|----------------|---------------|---------------|
|ns between ops        |  10.82         |  3.62         |  3.54         |
|ops per second        |  92,462,606    |  276,602,104  |  282,731,988  |
|timing spread (min)   |  10.75         |  3.45         |  3.44         |
|timing spread (med)   |  10.77         |  3.63         |  3.53         |
|timing spread (95%)   |  11.01         |  3.80         |  3.73         |
|timing spread (max)   |  11.07         |  3.80         |  3.73         |

### Lamda

|12 Runs, 500k batch   | Debug          | Release       | Distrubtion   |
|----------------------|----------------|---------------|---------------|
|ns between ops        |  8.21          |  0.23         |  0.25         |
|ops per second        |  121,872,943   | 4,378,603,226 | 3,986,710,963 |
|timing spread (min)   |  7.96          |  0.20         |  0.25         |
|timing spread (med)   |  8.03          |  0.24         |  0.25         |
|timing spread (95%)   |  8.82          |  0.25         |  0.25         |
|timing spread (max)   |  8.96          |  0.25         |  0.28         |