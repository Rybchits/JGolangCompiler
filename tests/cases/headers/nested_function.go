package main
func main() {
    if (func() bool {
        if true { return true }
        return false
    })() { println(1) }
}
