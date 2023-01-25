# SO2-Pr-Threads
Small project to explore the problems of threads' synchonization with C++.
Threads' actions are visualized using ncurses.

# Topic: Cardiovascular system
- Threads:
	- lungs - inhale / exhale cycles - generating oxygen and transferring it to erythrocytes
	- heart - moves blood cells
	- erythrocytes - oxygen transportation from lungs to body cells
	- leukocytes - "patrolling" body to fight bacteria
				- helping body cell attacked by a bacteria
	- bacteria - randomly attack body cells
	- body cells - to each body cell eruthrocytes with oxygen must arrive
			- can be attacked by bacteria, then leukocytes must arrive
- Resources:
	- oxygen - one unit of oxygen can be taken from lungs by single erythrocyte
	- blood vessels - constrained number of blood cells - they must not collide
			- forces blood cells dispersion in the vascular system
- Number of threads:
	- lungs - one
	- heart - one
	- erythrocytes - a few
	- leukocyty - a few
	- bakterie - a few
	- komórki - a few
- System descriptions:
	- System is made of lungs, heart and vessels.
	- Vessels are connecting lungs with body cells to create closed circulatory cycle.
	- Each body cell require regular oxygen supply.
	- Erythrocytes, which are cirulating in vessels, are responsible for the oxygen transport.
	- Each erythrocyte can take one unit of oxygen from lungs and transfer it to a single body cell.
	- Body cells can be attacked by bacteria - it requires the intervention of leukocytes which are circulating in the vessels with erythrocytes.
