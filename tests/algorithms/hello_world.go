package main

func main() {
	const hello = "Hello "
	const world = "world"
	printString(hello + world)
	printRune(':')
	printInt(lenString(hello + world))
}
