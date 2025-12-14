#include <expected>
#include <utility> // <-- Add this line
using Result = std::expected<int, int>;

Result doIt(){
  return std::unexpected(5);
}

int main () {
	return 1;
}
