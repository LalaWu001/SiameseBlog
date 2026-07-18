# 浏览器写作后台

SiameseBlog 使用 Pages CMS 编辑 GitHub 仓库中的 Markdown 内容。CMS 不保存独立数据库；保存文章时会直接提交文件和图片，随后由现有 Vercel 流程重新构建静态网站。

## 首次连接

1. 确保包含 `.pages.yml` 的版本已经推送到 GitHub。
2. 打开 [Pages CMS](https://app.pagescms.org/) 并使用 GitHub 登录。
3. 安装 Pages CMS GitHub App，只授权 `LalaWu001/SiameseBlog` 仓库。
4. 在 Pages CMS 中打开该仓库和生产分支。
5. 左侧会显示“博客文章”“动态”“微醺小记”三个内容入口。

Pages CMS 按 GitHub 仓库权限决定谁能写入。不要邀请其他仓库协作者，即可保持只有仓库所有者能够发布。不要把 GitHub Token、Cookie 或 Pages CMS 凭据写进仓库。

## 发布流程

1. 新建内容或打开现有内容。
2. 填写必填字段并上传图片。
3. 未完成时打开“草稿”开关。
4. 准备发布时关闭“草稿”开关并保存。
5. Pages CMS 提交到 GitHub 后，等待 Vercel 构建完成。

草稿不会进入博客、动态或微醺小记的公开列表和详情路由。

## 图片位置

- 封面和相册图片：保存到 `src/assets/uploads/`，由 Astro 在构建时优化。
- 正文插图：保存到 `public/uploads/`，Markdown 通过 `/uploads/...` 直接引用。
- 微醺小记写作模板：`src/content/drinks/写作模板.md`，默认是草稿，不会公开显示。

上传前建议压缩原图，并为封面和相册图片填写有意义的替代文字。

## 微醺小记字段

- `title`：酒名或酒评标题。
- `description`：列表卡片上的一句话评价。
- `drinkTypes`：可添加多个标签，例如“威士忌”“啤酒”。
- `rating`：0 到 5 分，支持 0.5 分。
- `cover`：列表和详情页主图。
- `images`：详情页相册，可添加多张。
- `tastedAt`、`brand`、`origin`、`alcoholByVolume`：品饮信息。
- `body`：完整评价正文。
- `draft`：打开时仅保存在仓库，不公开生成。

后台地址不需要出现在博客顶栏。隐藏地址不是安全措施，真正的发布权限由 GitHub 仓库写权限控制。
