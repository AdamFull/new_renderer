#include <parsing/argparse.h>

int main(int argc, char** argv)
{
	utl::parsing::argument_parser argparse{};

	argparse.parse_arguments(argc, argv);



	return 0;
}