#include <string>

#include "Replayer/NasdaqReplayer.hpp"

int main() {

	// Przykładowo: analizujemy microsofta

	const std::string file_name = "/home/misha/Documents/binary nasdaq itch/01302020.NASDAQ_ITCH50";

	const auto replayer = new Replayer::NasdaqReplayer(file_name);

	replayer->Run();

	return 0;

}