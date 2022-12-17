package loops

func main() {
    sum := 0
    i := 1
    for ; ; {
    	if i%2 != 0 {
    		continue
    	}
    	sum += i
    	i++
    }
    println(sum)
}