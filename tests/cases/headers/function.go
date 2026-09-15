package main
func main() {
    if func() bool { return true }() { println(1) }
    for func() bool { return true }() { break }
}
