# Prepend directories of TAO software in PATH and LD_LIBRARY_PATH.
# Created on Mon 25 Apr 2022 02:44:59 PM +07 by ./install-tao-profile

dir="$HOME/local_tao/bin"
case ":$PATH:" in
  ::)         export PATH="$dir" ;;
  *:"$dir":*) : do nothing ;;
  *)          export PATH="$dir:$PATH" ;;
esac

dir="$HOME/local_tao/lib"
case ":$LD_LIBRARY_PATH:" in
  ::)         export LD_LIBRARY_PATH="$dir" ;;
  *:"$dir":*) : do nothing ;;
  *)          export LD_LIBRARY_PATH="$dir:$PATH" ;;
esac

# For julia packages:
export TAO_INCDIR=$HOME/local_tao/include
export TAO_LIBDIR=$HOME/local_tao/lib

