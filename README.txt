Pujeet K
Multithreaded Sports Field

This project creates an executable, "stadium," which simulates the use of Worcester Gompei Park by 3 sports and 140 players. There are 36 baseball players (4 teams, 9 per team),
44 football players (4 teams, 11 per team), and 60 rugby players (4-60 teams, 1-15 per team). There are certain restrictions for the use of the field which the project
implements, and they are as follows:

1. The field must support these 3 sports and their players
2. The matches should support the designated number of players per team
3. Only one sport can be on the field (playing) at a time
4. The maximum number of players per team (in the case of rugby) should try to be achieved each time
5. Each sport/player should have a chance to play without waiting forever
6. Football and baseball must be played for a set amount of time, the rugby pairs must play for a set amount of time
7. If a player wasn't able to join a team/match, it must sleep instead of busy-waiting

NOTES ABOUT PROJECT:
The way I have my solution for this problem set up, rugby players are able to get the maximum number of players per team (15) and on the field (30) each time they play. Therefore,
there should be no instance of a rugby match with less than 30 players. I have it set up like this because of the way I solved the 3rd restriction (keeping track of last played
sport). The fairness still exists, and restriction 4 is met too (all restrictions should be met), so I figured setting it up this way would be okay.