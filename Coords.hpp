struct Coords {
	int line = 0;
	int col = 0;
	Coords() = default;
	Coords(int line, int col): line(line), col(col){};
	~Coords() = default;
};
