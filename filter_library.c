#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <float.h>

#define bad_malloc(size)						\
	err(1, "%s [Line: %d] misallocation of %zu bytes",	\
			__func__, __LINE__, size)
#define PR_MOVE 0.9
#define PR_STAY 0.1
#define PR_CORRECT_INFO 0.9
#define PR_INCORRECT_INFO 0.1
#define PR_ALT 0.005


enum info {
    N,
    H,
    T,
    B,
};

struct node {
	int x;
	int y;
	enum info type;
};

struct world_data {

	struct node **nodes;

	unsigned int num_rows;
	unsigned int num_cols;


	unsigned int size;


	unsigned int num_unblocked_nodes;


	unsigned int loaded;
};

static struct world_data curr_world;


static enum info char_to_type(char c)
{
	switch (c) {
	case 'N':
		return N;
	case 'H':
		return H;
	case 'T':
		return T;
	case 'B':
		return B;
	default:
		errx(-1, "Passed in bad value (%d) to %s\n", c, __func__);
	}
}
static inline int is_valid_info(char c)
{
	return c == 'N' || c == 'H' || c == 'T' || c == 'B';
}

static inline int is_valid_action(char c)
{
	return c == 'U' || c == 'D' || c == 'L' || c == 'R';
}


static inline int type_delta_x(char a)
{
	switch (a) {
	case 'U':
		return -1;
	case 'D':
		return 1;
	default:
		return 0;
	}
}


static inline int type_delta_y(char a)
{
	switch (a) {
	case 'L':
		return -1;
	case 'R':
		return 1;
	default:
		return 0;
	}
}


static void point_mult(double *v1, double *v2, int v1_len, int v2_len)
{
	int i;

	if (v1_len == v2_len) {
		for (i = 0; i < v1_len; i++)
			v1[i] *= v2[i];
	} else if (v1_len == 1) {
		for (i = 0; i < v2_len; i++)
			v1[0] *= v2[i];
	} else if (v2_len == 1) {
		double v2_val = *v2;
		for (i = 0; i < v1_len; i++)
			*v1++ *= v2_val;
	} else {
		errx(1, "Gave bad input to point mult"
			" params (v1_len, v2_len) = (%d, %d)",
			v1_len, v2_len);
	}
}



static void parse_world_file(const char *file)
{
	struct node **world, **world_nodes, *row;
	FILE *fp;
	char buf[4096];
	char *start, *end;
	unsigned int nodes_read = 0, num_unblocked = 0, num_rows, num_cols;
	int i = 0, so_far = 0;

	fp = fopen(file, "r");
	if (!fp)
		err(1, "Error opening %s", file);


	if (!fgets(buf, sizeof(buf), fp))
		err(1, "Failed to read first line of file for size. "
			"Ensure correct format.");
	start = buf;
	num_rows = strtol(start, &end, 10);
	if (num_rows == LONG_MIN || num_rows == LONG_MAX || start == end)
		err(1, "Failed to read in number of rows from file. "
			"Contents of line:\n\t%s", buf);
	start = end;
	num_cols = strtol(start, &end, 10);
	if (num_cols == LONG_MIN || num_cols == LONG_MAX || start == end)
		err(1, "Failed to read in number of columns from file. "
			"Contents of line:\n\t%s", buf);

	world = malloc(sizeof(*world) * num_rows);
	if (!world)
		bad_malloc(sizeof(*world) * num_rows);
	world_nodes = world;
	row = malloc(sizeof(*row) * num_cols);
	if (!row)
		bad_malloc(sizeof(*row) * num_cols);


	while (fgets(buf, sizeof(buf), fp)) {
		struct node *n = &row[i];
		int row_num, col_num;
		char type;
		start = buf;
		row_num = strtol(start, &end, 10);
		if (row_num == LONG_MIN || row_num == LONG_MAX || start == end)
			err(1, "Failed to read in x coordinate of node. "
				"Contents of line:\n\t%s", buf);
		start = end;
		col_num = strtol(start, &end, 10);
		if (col_num == LONG_MIN || col_num == LONG_MAX || start == end)
			err(1, "Failed to read in y coordinate of node. "
				"Contents of line:\n\t%s", buf);

		type = *++end;
		if (!is_valid_info(type))
			errx(1, "Failed to retrieve valid terrain type from line. "
				"Got %c (val: %d) from line:\n\t%s",
				type, type, buf);

		n->x = row_num;
		n->y = col_num;
		n->type = char_to_type(type);
		if (n->type != B)
			num_unblocked++;
		i++;
		nodes_read++;

	
		if (++so_far == num_cols) {
			*world++ = row;
			row = malloc(sizeof(*row) * num_cols);
			if (!row)
				bad_malloc(sizeof(*row) * num_cols);
			i = 0;
			so_far = 0;
		}
	}
	if (nodes_read != num_rows * num_cols)
		errx(1, "Read only %u nodes, when file said to expect %d cells.",
				nodes_read, num_rows * num_cols);
	if (fclose(fp))
		warn("Error closing world file! Be cautious opening more files.");
	curr_world.nodes = world_nodes;
	curr_world.num_rows = num_rows;
	curr_world.num_cols = num_cols;
	curr_world.size = num_rows * num_cols;
	curr_world.num_unblocked_nodes = num_unblocked;
}
static double *init_distr(int num_unblocked)
{
	double *distr, val;
	unsigned int size = curr_world.size;
	int i;

	distr = malloc(sizeof(*distr) * size);
	if (!distr)
		bad_malloc(sizeof(*distr) * size);
	val = 1.0 / num_unblocked;
	for (i = 0; i < size; i++)
		distr[i] = val;
	return distr;
}


static inline int in_bounds(int row, int col)
{
	unsigned int num_rows = curr_world.num_rows;
	unsigned int num_cols = curr_world.num_cols;
	return row >= 0 && row < num_rows && col >= 0 && col < num_cols;
}


struct trans_result {
	double res1;
	double res2;
	int pos1;
	int pos2;
};


static void transition(struct trans_result *tr, struct node **world,
			struct node *c, char action)
{
	unsigned int num_cols = curr_world.num_cols;
	int destx, desty;

	destx = c->x + type_delta_x(action);
	desty = c->y + type_delta_y(action);

	if (!in_bounds(destx - 1, desty - 1) || world[destx - 1][desty - 1].type == B) {
	
		tr->res1 = 1.0;
		tr->pos1 = (num_cols * (c->x - 1)) + (c->y - 1);
		tr->res2 = 0.0;
		tr->pos2 = 0;
	} else {
		tr->res1 = PR_STAY;
		tr->pos1 = (num_cols * (c->x - 1)) + (c->y - 1);
		tr->res2 = PR_MOVE;
		tr->pos2 = (num_cols * (destx - 1)) + (desty - 1);
	}
}


static void scalar_mult(struct trans_result *tr, double t)
{
	tr->res1 *= t;
	tr->res2 *= t;
}


static void add_scaled_trans(struct trans_result *tr, double *sum)
{
	sum[tr->pos1] += tr->res1;
	sum[tr->pos2] += tr->res2;
}


static void observation(struct node **world, double *obs, enum info sensed)
{
	unsigned int size = curr_world.size;
	unsigned int num_cols = curr_world.num_cols;
	int i;

	for (i = 0; i < size; i++) {
		struct node *n = &world[i / num_cols][i % num_cols];
	
		if (n->type == B)
			obs[i] = 0.0;
		else if (n->type == sensed)
			obs[i] = PR_CORRECT_INFO;
		else
			obs[i] = PR_ALT;
	}
}

static void print_vector(double *p, unsigned long size)
{
	unsigned long i;

	printf("[ ");
	for (i = 0; i < size; i++)
		printf("%g, ", p[i]);
	printf(" ]\n");
}


static double *do_filter(struct node **world, const char sensor,
		const char action, double *prev)
{
	double *summation, *trans_model, *obs;
	double alpha;
	unsigned int size = curr_world.size;
	unsigned int num_cols = curr_world.num_cols;
	int i;


	if (!world || !prev || !is_valid_info(sensor) || !is_valid_action(action))
		return NULL;


	trans_model = calloc(size, sizeof(double));
	if (!trans_model)
		bad_malloc(size * sizeof(double));


	obs = calloc(size, sizeof(double));
	if (!obs)
		bad_malloc(size * sizeof(double));


	summation = calloc(size, sizeof(double));
	if (!summation)
		bad_malloc(size * sizeof(double));


	for (i = 0; i < size; i++) {
		struct node *curr = &world[i / num_cols][i % num_cols];
		struct trans_result tr;

	
		if (curr->type == B)
			continue;
		memset(&tr, 0, sizeof(tr));

	
		transition(&tr, world, curr, action);
	
		scalar_mult(&tr, prev[i]);
	
		add_scaled_trans(&tr, summation);
	}

	observation(world, obs, char_to_type(sensor));

	point_mult(summation, obs, size, size);


	alpha = 0;
	for (i = 0; i < size; i++)
		alpha += summation[i];
	alpha = 1.0 / alpha;


	for (i = 0; i < size; i++)
		summation[i] *= alpha;

	free(obs);
	free(trans_model);
	return summation;
}


static void unload_world(void)
{
	unsigned int num_rows = curr_world.num_rows;
	int i;

	for (i = 0; i < num_rows; i++)
		free(curr_world.nodes[i]);
	free(curr_world.nodes);
	curr_world.loaded = 0;
}


void free_buffer(void *buf)
{
	free(buf);
}


unsigned int load_world(const char *world_file)
{
	if (curr_world.loaded)
		unload_world();
	parse_world_file(world_file);
	curr_world.loaded = 1;
	return curr_world.size;
}


double *filter_step(char sensor_data, char action, double *prev)
{
	double *res;

	if (!curr_world.loaded)
		errx(1, "Call load_world() before trying to do filter_step!");
	if (!prev)
		prev = init_distr(curr_world.num_unblocked_nodes);
	res = do_filter(curr_world.nodes, sensor_data, action, prev);
	free(prev);
	return res;
}