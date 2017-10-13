$includes = {}

Dir.glob("../**/{*.h,*.cpp}").select { |e| 
    text = File.open(e).readlines

    text.each { |line| 
        matches = line.match(/^\#include\s*<([^>]*)>\s*$/)

        if !matches.nil?
            matches.captures.each { |match|
                if !$includes.key?(match)
                    $includes[match] = true                                    
                end
            }
        end
    }
}

sorted = $includes.keys.sort_by(&:downcase)

sorted.each { |s|

    puts "#include <#{s}>"
}
