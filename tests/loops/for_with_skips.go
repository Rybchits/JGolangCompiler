package loops

func main() {
    sum := 0
    i := 1
    for ; i < 5; {
    	if i%2 != 0 {
    		continue
    	}
    	sum += i
    	i++
    }
    println(sum)
}