class Hatools < Formula
  desc "The halockrun and hatimerun command line tools"
  homepage "https://github.com/fatalmind/hatools"
  url "https://fatalmind.com/software/hatools/hatools-2.14.tar.gz"
  sha256 "4bad4723056f87f30633a4702b96be3b2e0d1218b7b96a79c71f42ca3f5dd109"
  license "GPL-2.0-or-later"

  depends_on "make" => :build

  def install
    system "./configure", *std_configure_args, "--disable-silent-rules"
    system "make", "install"
  end

  test do
    system "#{bin}/halockrun","-c","test.lck","echo","ok" && assert_equal($?.exitstatus, 0)
  end
end
