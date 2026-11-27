import ifj26.*

def sumTo(limit: Int): Int =
  var total = 0
  var i = 1
  while i <= limit do
    total = total + i
    i = i + 1
  total

@main def controlFlowExample(): Unit =
  val limit = readInt()
  if limit >= 0 then
    val total = sumTo(limit)
    print("sum: ")
    print(total)
    print("\n")
  else
    print("limit must be nonnegative\n")
